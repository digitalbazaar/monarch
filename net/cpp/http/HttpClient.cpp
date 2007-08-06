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
      char* field;
      char* colon;
      for(int i = 0; headers[i] != NULL; i++)
      {
         // find colon
         field = headers[i];
         if((colon = strchr(field, ':')) != NULL)
         {
            // get field name
            char* name = new char[colon - field + 1];
            strncpy(name, field, colon - field);
            memset(name + (colon - field), 0, 1);
            
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

HttpResponse* HttpClient::post(Url* url, char** headers, InputStream* is)
{
   HttpResponse* rval = NULL;
   
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
      if(mRequest->sendHeader() == NULL)
      {
         // send body
         if(mRequest->sendBody(is) == NULL)
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
   
   // FIXME: add SSL support later
   
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
