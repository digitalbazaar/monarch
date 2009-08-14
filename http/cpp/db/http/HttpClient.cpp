/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/http/HttpClient.h"

#include "db/net/TcpSocket.h"
#include "db/net/SslSocket.h"
#include "db/io/InputStream.h"
#include "db/io/OutputStream.h"
#include "db/rt/DynamicObjectIterator.h"

using namespace db::io;
using namespace db::http;
using namespace db::net;
using namespace db::rt;

HttpClient::HttpClient(SslContext* sc) :
   mConnection(NULL),
   mRequest(NULL),
   mResponse(NULL),
   mSslContext(sc),
   mCleanupSslContext(sc == NULL)
{
}

HttpClient::~HttpClient()
{
   // ensure client is disconnected
   HttpClient::disconnect();

   if(mCleanupSslContext && mSslContext != NULL)
   {
      delete mSslContext;
   }
}

bool HttpClient::connect(Url* url)
{
   if(mConnection == NULL)
   {
      // create connection as necessary
      InternetAddress address(url->getHost().c_str(), url->getPort());

      SslContext* ssl = NULL;
      if(strcmp(url->getScheme().c_str(), "https") == 0)
      {
         // create ssl context if necessary
         if(mSslContext == NULL)
         {
            mSslContext = new SslContext(NULL, true);
         }

         ssl = mSslContext;
      }

      if((mConnection = createConnection(
         &address, ssl, &mSslSession, 30)) != NULL)
      {
         // store ssl session if appropriate
         if(ssl != NULL)
         {
            mSslSession = ((SslSocket*)mConnection->getSocket())->getSession();
         }

         // create request and response
         mRequest = (HttpRequest*)mConnection->createRequest();
         mResponse = (HttpResponse*)mRequest->createResponse();

         // set default timeouts
         mConnection->setReadTimeout(30000);
         mConnection->setWriteTimeout(30000);
      }
   }

   return mConnection != NULL;
}

HttpResponse* HttpClient::get(Url* url, DynamicObject* headers, bool follow)
{
   HttpResponse* rval = NULL;

   // ensure connected
   if(connect(url))
   {
      // set request header
      mRequest->getHeader()->setMethod("GET");
      mRequest->getHeader()->setPath(url->getPathAndQuery().c_str());
      mRequest->getHeader()->setVersion("HTTP/1.1");
      mRequest->getHeader()->clearFields();
      mRequest->getHeader()->setField("Host", url->getAuthority());
      mRequest->getHeader()->setField("User-Agent", "DB Http Client/2.0");

      if(headers != NULL)
      {
         // set custom headers
         setCustomHeaders(mRequest->getHeader(), *headers);
      }

      // send request header and receive response header
      if(mRequest->sendHeader() && mResponse->receiveHeader())
      {
         // handle http redirect
         int code = mResponse->getHeader()->getStatusCode();
         if(follow && (code == 301 || code == 302))
         {
            // get new location
            std::string location;
            if(!mResponse->getHeader()->getField("location", location))
            {
               ExceptionRef e = new Exception(
                  "Got redirect response code, but could not redirect. "
                  "No location field in header.",
                  "db.net.http.InvalidRedirect");
               e->getDetails()["statusCode"] = code;
               Exception::set(e);
            }
            else
            {
               // disconnect and then do get to redirect url
               disconnect();
               Url redirect = location.c_str();
               rval = get(&redirect, headers, follow);
            }
         }
         else
         {
            // return response
            rval = mResponse;
         }
      }
   }

   return rval;
}

HttpResponse* HttpClient::post(
   Url* url, DynamicObject* headers, InputStream* is, HttpTrailer* trailer,
   bool skipContinue)
{
   HttpResponse* rval = NULL;

   // ensure connected
   if(connect(url))
   {
      // set request header
      mRequest->getHeader()->setMethod("POST");
      mRequest->getHeader()->setPath(url->getPathAndQuery().c_str());
      mRequest->getHeader()->setVersion("HTTP/1.1");
      mRequest->getHeader()->clearFields();
      mRequest->getHeader()->setField("Host", url->getAuthority());
      mRequest->getHeader()->setField("User-Agent", "DB Http Client/2.0");

      if(headers != NULL)
      {
         // set custom headers
         setCustomHeaders(mRequest->getHeader(), *headers);
      }

      // send request header, send body, and receive response header
      if(mRequest->sendHeader() &&
         mRequest->sendBody(is, trailer) &&
         mResponse->receiveHeader())
      {
         // receive and throw-out 100 continue
         if(skipContinue && mResponse->getHeader()->getStatusCode() == 100)
         {
            if(mResponse->receiveHeader())
            {
               // return response
               rval = mResponse;
            }
         }
         else
         {
            // return response
            rval = mResponse;
         }
      }
   }

   return rval;
}

bool HttpClient::receiveContent(OutputStream* os, HttpTrailer* trailer)
{
   bool rval = false;

   if(mConnection == NULL)
   {
      ExceptionRef e = new Exception(
         "Could not receive HTTP content, not connected.",
         "db.net.http.NotConnected");
      Exception::set(e);
   }
   else
   {
      // receive body
      rval = mResponse->receiveBody(os, trailer);
   }

   return rval;
}

void HttpClient::disconnect()
{
   if(mRequest != NULL)
   {
      delete mRequest;
      mRequest = NULL;
   }

   if(mResponse != NULL)
   {
      delete mResponse;
      mResponse = NULL;
   }

   if(mConnection != NULL)
   {
      mConnection->close();
      delete mConnection;
      mConnection = NULL;
   }
}

HttpConnection* HttpClient::createConnection(
   Url* url, SslContext* context, SslSession* session,
   unsigned int timeout, DynamicObject* commonNames)
{
   // create connection
   InternetAddress address(url->getHost().c_str(), url->getPort());
   return createConnection(&address, context, session, timeout, commonNames);
}

HttpConnection* HttpClient::createSslConnection(
   Url* url, SslContext& context, SslSessionCache& cache,
   unsigned int timeout, DynamicObject* commonNames)
{
   HttpConnection* rval;

   // get existing ssl session
   SslSession session = cache.getSession(url);

   // create ssl connection
   rval = createConnection(
      url, &context, (session.isNull() ? NULL : &session), timeout,
      commonNames);
   if(rval != NULL)
   {
      // store session
      session = ((SslSocket*)rval->getSocket())->getSession();
      cache.storeSession(url, session);
   }

   return rval;
}

HttpConnection* HttpClient::createConnection(
   InternetAddress* address, SslContext* context, SslSession* session,
   unsigned int timeout, DynamicObject* commonNames)
{
   HttpConnection* rval = NULL;

   // connect with given timeout
   Socket* s = new TcpSocket();
   if(s->connect(address, timeout))
   {
      // do SSL if appropriate
      if(context != NULL)
      {
         // create ssl socket, reuse passed session
         SslSocket* ss;
         ss = new SslSocket(context, (TcpSocket*)s, true, true);
         s = ss;
         ss->setSession(session);

         // no special common names, so use default: url host
         if(commonNames == NULL)
         {
            ss->addVerifyCommonName(address->getHost());
         }
         else if((*commonNames)->getType() == Array)
         {
            // only add common names (and host) if length of list > 0
            if((*commonNames)->length() > 0)
            {
               // always add host
               ss->addVerifyCommonName(address->getHost());

               // add all common names to be checked
               DynamicObjectIterator i = commonNames->getIterator();
               while(i->hasNext())
               {
                  DynamicObject& next = i->next();
                  ss->addVerifyCommonName(next->getString());
               }
            }
         }

         // start ssl session
         ss->performHandshake();
      }

      rval = new HttpConnection(new Connection(s, true), true);
   }
   else
   {
      // close and clean up socket
      s->close();
      delete s;
   }

   return rval;
}

void HttpClient::setCustomHeaders(HttpHeader* h, DynamicObject& headers)
{
   if(!headers.isNull())
   {
      DynamicObjectIterator i = headers.getIterator();
      while(i->hasNext())
      {
         DynamicObject& value = i->next();
         const char* field = i->getName();
         if(value->getType() == Array)
         {
            DynamicObjectIterator ai = value.getIterator();
            while(ai->hasNext())
            {
               DynamicObject& av = ai->next();
               h->addField(field, av->getString());
            }
         }
         else
         {
            h->addField(field, value->getString());
         }
      }
   }
}
