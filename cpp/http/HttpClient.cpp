/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpClient.h"

#include "monarch/net/TcpSocket.h"
#include "monarch/net/SslSocket.h"
#include "monarch/io/ByteArrayOutputStream.h"
#include "monarch/io/InputStream.h"
#include "monarch/io/OutputStream.h"
#include "monarch/rt/DynamicObjectIterator.h"

using namespace monarch::io;
using namespace monarch::http;
using namespace monarch::net;
using namespace monarch::rt;

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

   if(mCleanupSslContext)
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
            mSslSession = (static_cast<SslSocket*>(mConnection->getSocket()))->
               getSession();
         }

         // create request and response
         mRequest = mConnection->createRequest();
         mResponse = mRequest->createResponse();

         // set default timeouts
         mConnection->setReadTimeout(30000);
         mConnection->setWriteTimeout(30000);
      }
   }

   return mConnection != NULL;
}

SocketAddress* HttpClient::getLocalAddress()
{
   return (mConnection != NULL) ? mConnection->getLocalAddress() : NULL;
}

SocketAddress* HttpClient::getRemoteAddress()
{
   return (mConnection != NULL) ? mConnection->getRemoteAddress() : NULL;
}

HttpResponse* HttpClient::get(
   Url* url, DynamicObject* headers, int maxRedirects)
{
   mRedirectList.clear();
   return getRecursive(url, headers, maxRedirects);
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
      mRequest->getHeader()->setField("User-Agent", "Monarch Http Client/2.0");

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
         "monarch.net.http.NotConnected");
      Exception::set(e);
   }
   else
   {
      // receive body
      rval = mResponse->receiveBody(os, trailer);
   }

   return rval;
}

bool HttpClient::receiveContent(std::string& str, HttpTrailer* trailer)
{
   bool rval = true;

   int64_t length;
   int size = 2048;
   if(mResponse != NULL &&
      mResponse->getHeader()->getField("Content-Length", length))
   {
      if(length > 0x7FFFFFFF)
      {
         ExceptionRef e = new Exception(
            "Could not receive HTTP content, content too large.",
            "monarch.net.http.NotConnected");
         e->getDetails()["Content-Length"] = length;
         Exception::set(e);
         rval = false;
      }
      else
      {
         size = (int)length;
      }
   }

   if(rval)
   {
      ByteBuffer buf(size);
      ByteArrayOutputStream baos(&buf, true);
      rval = mResponse->receiveBody(&baos, trailer);
      if(rval)
      {
         str.assign(buf.data(), buf.length());
      }
   }

   return rval;
}

void HttpClient::disconnect()
{
   delete mRequest;
   mRequest = NULL;

   delete mResponse;
   mResponse = NULL;

   if(mConnection != NULL)
   {
      mConnection->close();
      delete mConnection;
      mConnection = NULL;
   }
}

HttpConnection* HttpClient::createConnection(
   Url* url, SslContext* context, SslSession* session,
   unsigned int timeout, DynamicObject* commonNames, bool includeHost,
   const char* vHost)
{
   // create connection
   InternetAddress address(url->getHost().c_str(), url->getPort());
   return createConnection(
      &address, context, session, timeout, commonNames, includeHost, vHost);
}

HttpConnection* HttpClient::createSslConnection(
   Url* url, SslContext& context, SslSessionCache& cache,
   unsigned int timeout, DynamicObject* commonNames, bool includeHost,
   const char* vHost)
{
   HttpConnection* rval;

   // get existing ssl session
   SslSession session = cache.getSession(url);

   // create ssl connection
   rval = createConnection(
      url, &context, (session.isNull() ? NULL : &session), timeout,
      commonNames, includeHost, vHost);
   if(rval != NULL)
   {
      // store session
      session = static_cast<SslSocket*>(rval->getSocket())->getSession();
      cache.storeSession(url, session, vHost);
   }

   return rval;
}

HttpConnection* HttpClient::createConnection(
   InternetAddress* address, SslContext* context, SslSession* session,
   unsigned int timeout, DynamicObject* commonNames, bool includeHost,
   const char* vHost)
{
   HttpConnection* rval = NULL;

   // connect with given timeout
   Socket* s = new TcpSocket();
   if(s->connect(address, timeout))
   {
      // do non-SSL
      if(context == NULL)
      {
         rval = new HttpConnection(new Connection(s, true), true);
      }
      // do SSL
      else
      {
         // create ssl socket, reuse passed session
         SslSocket* ss;
         ss = new SslSocket(context, static_cast<TcpSocket*>(s), true, true);
         s = ss;
         ss->setSession(session);

         // use virtual host, if provided
         if(vHost != NULL)
         {
            ss->setVirtualHost(vHost);
         }

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
               // add host if applicable
               if(includeHost)
               {
                  ss->addVerifyCommonName(address->getHost());
               }

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
         if(ss->performHandshake())
         {
            rval = new HttpConnection(new Connection(s, true), true);
         }
      }
   }

   if(rval == NULL)
   {
      ExceptionRef e = new Exception(
         "Could not establish HTTP connection.",
         "monarch.http.ConnectError");
      e->getDetails()["address"] = address->toString(false).c_str();
      Exception::push(e);

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

HttpResponse* HttpClient::getRecursive(
   Url* url, DynamicObject* headers, int maxRedirects)
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
      mRequest->getHeader()->setField("User-Agent", "Monarch Http Client/2.0");

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
         if(maxRedirects > 0 && (code == 301 || code == 302))
         {
            // get new location
            std::string location;
            if(!mResponse->getHeader()->getField("location", location))
            {
               ExceptionRef e = new Exception(
                  "Got redirect response code, but could not redirect. "
                  "No location field in header.",
                  "monarch.net.http.InvalidRedirect");
               e->getDetails()["statusCode"] = code;
               Exception::set(e);
            }
            else
            {
               // ensure location is not in the redirect list
               RedirectList::iterator i = std::find(
                  mRedirectList.begin(), mRedirectList.end(), location);
               if(i != mRedirectList.end())
               {
                  ExceptionRef e = new Exception(
                     "Got redirect response code, but could not redirect. "
                     "Redirect loop detected.",
                     "monarch.net.http.InvalidRedirect");
                  e->getDetails()["statusCode"] = code;
                  Exception::set(e);
               }
               else
               {
                  // add location to redirect list
                  mRedirectList.push_back(location);

                  // disconnect and then do get to redirect url
                  disconnect();
                  Url redirect = location.c_str();
                  rval = get(&redirect, headers, --maxRedirects);
               }
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
