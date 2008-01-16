/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpClient.h"
#include "db/net/TcpSocket.h"
#include "db/net/SslSocket.h"
#include "db/io/InputStream.h"
#include "db/io/OutputStream.h"

using namespace db::io;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;

HttpClient::HttpClient()
{
   mConnection = NULL;
   mRequest = NULL;
   mResponse = NULL;
   mSslContext = NULL;
}

HttpClient::~HttpClient()
{
   // ensure client is disconnected
   HttpClient::disconnect();
}

void HttpClient::setHeaders(HttpHeader* h, const char** headers)
{
   if(headers != NULL)
   {
      // go through headers until NULL is reached
      const char* field;
      char* colon;
      for(int i = 0; headers[i] != NULL; i++)
      {
         // find colon
         field = headers[i];
         if((colon = strchr(field, ':')) != NULL)
         {
            // get field name
            char name[colon - field + 1];
            strncpy(name, field, colon - field);
            name[(colon - field)] = 0;
            
            // skip whitespace
            colon++;
            for(; *colon == ' ' && *colon != 0; colon++);
            
            // set field
            h->setField(name, colon);
         }
      }
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
         &address, 30, ssl, &mSslSession)) != NULL)
      {
         // store ssl session if appropriate
         if(ssl != NULL)
         {
            mSslSession = ((SslSocket*)mConnection->getSocket())->getSession();
         }
         
         // create request and response
         mRequest = (HttpRequest*)mConnection->createRequest();
         mResponse = (HttpResponse*)mRequest->createResponse();
      }
   }
   
   return mConnection != NULL;
}

HttpResponse* HttpClient::get(Url* url, const char** headers)
{
   HttpResponse* rval = NULL;
   
   // ensure connected
   if(connect(url))
   {
      // set request header
      mRequest->getHeader()->setMethod("GET");
      mRequest->getHeader()->setPath(url->getPath().c_str());
      mRequest->getHeader()->setVersion("HTTP/1.1");
      mRequest->getHeader()->clearFields();
      mRequest->getHeader()->setField("Host", url->getAuthority());
      mRequest->getHeader()->setField("User-Agent", "DB Http Client/2.0");
      
      // set user headers
      setHeaders(mRequest->getHeader(), headers);
      
      // send request header
      if(mRequest->sendHeader() == NULL)
      {
         // receive response header
         if(mResponse->receiveHeader() == NULL)
         {
            // return response
            rval = mResponse;
         }
      }
   }
   
   return rval;
}

HttpResponse* HttpClient::post(
   Url* url, const char** headers, InputStream* is, HttpTrailer* trailer)
{
   HttpResponse* rval = NULL;
   
   // ensure connected
   if(connect(url))
   {
      // set request header
      mRequest->getHeader()->setMethod("POST");
      mRequest->getHeader()->setPath(url->getPath().c_str());
      mRequest->getHeader()->setVersion("HTTP/1.1");
      mRequest->getHeader()->clearFields();
      mRequest->getHeader()->setField("Host", url->getAuthority());
      mRequest->getHeader()->setField("User-Agent", "DB Http Client/2.0");
      
      // set user headers
      setHeaders(mRequest->getHeader(), headers);
      
      // send request header
      if(mRequest->sendHeader() == NULL)
      {
         // send body
         if(mRequest->sendBody(is, trailer) == NULL)
         {
            // receive response header
            if(mResponse->receiveHeader() == NULL)
            {
               // return response
               rval = mResponse;
            }
         }
      }
   }
   
   return rval;
}

IOException* HttpClient::receiveContent(OutputStream* os, HttpTrailer* trailer)
{
   IOException* rval = NULL;
   
   if(mConnection == NULL)
   {
      rval = new IOException("Could not receive HTTP content, not connected!");
      Exception::setLast(rval);
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
   Url* url, unsigned int timeout, SslContext* context, SslSession* session)
{
   // create connection
   InternetAddress address(url->getHost().c_str(), url->getPort());
   return createConnection(&address, timeout, context, session);
}

HttpConnection* HttpClient::createConnection(
   InternetAddress* address, unsigned int timeout,
   SslContext* context, SslSession* session)
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
