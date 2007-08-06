/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpClient.h"
#include "TcpSocket.h"
#include "InputStream.h"
#include "OutputStream.h"

using namespace db::io;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;

HttpClient::HttpClient()
{
   mConnection = NULL;
   mRequest = NULL;
   mResponse = NULL;
}

HttpClient::~HttpClient()
{
   // ensure client is disconnected
   HttpClient::disconnect();
}

void HttpClient::setHeaders(HttpHeader* h, char** headers)
{
   if(headers != NULL)
   {
      // go through headers until NULL is reached
      char* colon;
      for(int i = 0; headers[i] != NULL; i++)
      {
         // find colon
         if((colon = strchr(headers[i], ':')) != NULL)
         {
            // get field name
            char* name = new char[colon - headers[i] + 1];
            strncpy(name, 0, colon - headers[i]);
            memset(name + (colon - headers[i]), 0, 1);
            
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
      InternetAddress address(url->getHost(), url->getPort());
      if((mConnection = createConnection(&address)) != NULL)
      {
         // create request and response
         mRequest = (HttpRequest*)mConnection->createRequest();
         mResponse = (HttpResponse*)mRequest->createResponse();
      }
   }
   
   return mConnection != NULL;
}

HttpResponse* HttpClient::get(Url* url, char** headers)
{
   HttpResponse* rval = NULL;
   
   // ensure connected
   if(connect(url))
   {
      // set request header
      mRequest->getHeader()->setMethod("GET");
      mRequest->getHeader()->setPath(url->getPath());
      mRequest->getHeader()->setVersion("HTTP/1.1");
      mRequest->getHeader()->clearFields();
      mRequest->getHeader()->setField("Host", url->getAuthority());
      mRequest->getHeader()->setField("User-Agent", "DB Http Client v2.0");
      
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

bool HttpClient::post(Url* url, char** headers)
{
   bool rval = false;
   
   // ensure connected
   if(connect(url))
   {
      // set request header
      mRequest->getHeader()->setMethod("POST");
      mRequest->getHeader()->setPath(url->getPath());
      mRequest->getHeader()->setVersion("HTTP/1.1");
      mRequest->getHeader()->clearFields();
      mRequest->getHeader()->setField("Host", url->getAuthority());
      mRequest->getHeader()->setField("User-Agent", "DB Http Client v2.0");
      
      // set user headers
      setHeaders(mRequest->getHeader(), headers);
      
      // send request header
      rval = (mRequest->sendHeader() == NULL);
   }
   
   return rval;
}

IOException* HttpClient::sendContent(InputStream* is)
{
   IOException* rval = NULL;
   
   if(mConnection == NULL)
   {
      rval = new IOException("Could not send HTTP content, not connected!");
      Exception::setLast(rval);
   }
   else
   {
      // send body
      rval = mRequest->sendBody(is);
   }
   
   return rval;
}

IOException* HttpClient::receiveContent(OutputStream* os)
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
      rval = mResponse->receiveBody(os);
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
   InternetAddress* address, unsigned int timeout)
{
   HttpConnection* rval = NULL;
   
   // connect with given timeout
   TcpSocket* s = new TcpSocket();
   if(s->connect(address, timeout))
   {
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
