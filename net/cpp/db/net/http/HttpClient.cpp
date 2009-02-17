/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/net/http/HttpClient.h"

#include "db/net/TcpSocket.h"
#include "db/net/SslSocket.h"
#include "db/io/InputStream.h"
#include "db/io/OutputStream.h"
#include "db/rt/DynamicObjectIterator.h"

using namespace db::io;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;

HttpClient::HttpClient(SslContext* sc) :
   mConnection(NULL),
   mRequest(NULL),
   mResponse(NULL),
   mSslContext(NULL),
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

HttpResponse* HttpClient::get(Url* url, DynamicObject* headers)
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
         // return response
         rval = mResponse;
      }
   }
   
   return rval;
}

HttpResponse* HttpClient::post(
   Url* url, DynamicObject* headers, InputStream* is, HttpTrailer* trailer)
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
         // return response
         rval = mResponse;
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
      Exception::setLast(e, false);
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
   unsigned int timeout)
{
   // create connection
   InternetAddress address(url->getHost().c_str(), url->getPort());
   return createConnection(&address, context, session, timeout);
}

HttpConnection* HttpClient::createSslConnection(
   Url* url, SslContext& context, SslSessionCache& cache,
   unsigned int timeout)
{
   HttpConnection* rval;
   
   // get existing ssl session
   SslSession session = cache.getSession(url);
   
   // create ssl connection
   rval = createConnection(
      url, &context, session.isNull() ? NULL : &session, timeout);
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
   unsigned int timeout)
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
         s = new SslSocket(context, (TcpSocket*)s, true, true);
         ((SslSocket*)s)->setSession(session);
         
         // start ssl session
         ((SslSocket*)s)->performHandshake();
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
