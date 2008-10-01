/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpConnectionServicer.h"

#include "db/net/http/HttpRequest.h"
#include "db/net/http/HttpResponse.h"
#include "db/io/ByteArrayInputStream.h"

#include <cstdlib>

using namespace std;
using namespace db::io;
using namespace db::net::http;
using namespace db::rt;

HttpConnectionServicer::HttpConnectionServicer(const char* serverName)
{
   mServerName = strdup(serverName);
}

HttpConnectionServicer::~HttpConnectionServicer()
{
   free(mServerName);
}

HttpRequestServicer* HttpConnectionServicer::findRequestServicer(
   char* path, ServicerMap& servicerMap)
{
   HttpRequestServicer* rval = NULL;
   
   // strip any query
   if(path != NULL)
   {
      char* end = strrchr(path, '?');
      if(end != NULL)
      {
         end[0] = 0;
      }
   }
   
   mRequestServicerLock.lockShared();
   {
      // try to find servicer for path
      ServicerMap::iterator i;
      while(rval == NULL && path != NULL)
      {
         i = servicerMap.find(path);
         if(i != servicerMap.end())
         {
            rval = i->second;
         }
         else if(strlen(path) > 1)
         {
            // try to find servicer at parent path
            char* end = strrchr(path, '/');
            if(end != NULL)
            {
               end[0] = 0;
            }
            else
            {
               // no path left to search
               path = NULL;
            }
         }
         else
         {
            // no path left to search
            path = NULL;
         }
      }
   }
   mRequestServicerLock.unlockShared();
   
   return rval;
}

void HttpConnectionServicer::serviceConnection(Connection* c)
{
   // wrap connection, set default timeouts to 30 seconds
   HttpConnection hc(c, false);
   hc.setReadTimeout(30000);
   hc.setWriteTimeout(30000);
   
   // create request
   HttpRequest* request = (HttpRequest*)hc.createRequest();
   HttpRequestHeader* reqHeader = request->getHeader();
   
   // create response
   HttpResponse* response = (HttpResponse*)request->createResponse();
   HttpResponseHeader* resHeader = response->getHeader();
   
   // handle keep-alive (HTTP/1.1 keep-alive is on by default)
   bool keepAlive = true;
   bool noerror = true;
   while(keepAlive && noerror)
   {
      // set defaults
      resHeader->setVersion("HTTP/1.1");
      resHeader->setDate();
      resHeader->setField("Server", mServerName);
      
      // receive request header
      if((noerror = request->receiveHeader()))
      {
         // check http version
         bool version10 = (strcmp(reqHeader->getVersion(), "HTTP/1.0") == 0);
         bool version11 = (strcmp(reqHeader->getVersion(), "HTTP/1.1") == 0);
         
         // only version 1.0 and 1.1 supported
         if(version10 || version11)
         {
            // set response version according to request version
            resHeader->setVersion(reqHeader->getVersion());
            
            // include host path if one was used
            string host;
            if(reqHeader->getField("Host", host))
            {
               resHeader->setField("Host", host);
            }
            
            // get connection header
            string connHeader;
            if(reqHeader->getField("Connection", connHeader))
            {
               if(strcasecmp(connHeader.c_str(), "close") == 0)
               {
                  keepAlive = false;
               }
               else if(strcasecmp(connHeader.c_str(), "keep-alive") == 0)
               {
                  keepAlive = true;
               }
            }
            else if(version10)
            {
               // if HTTP/1.0 and no keep-alive header, keep-alive is off
               keepAlive = false;
            }
            
            // get request path and normalize it
            const char* inPath = reqHeader->getPath();
            char outPath[strlen(inPath) + 2];
            HttpRequestServicer::normalizePath(inPath, outPath);
            
            // find appropriate request servicer for path
            HttpRequestServicer* hrs = NULL;
            
            // find secure/non-secure servicer
            hrs = hc.isSecure() ?
               findRequestServicer(outPath, mSecureServicers) :
               findRequestServicer(outPath, mNonSecureServicers);
            
            if(hrs != NULL)
            {
               // service request
               hrs->serviceRequest(request, response);
               
               // if servicer closed connection, turn off keep-alive
               if(c->isClosed())
               {
                  keepAlive = false;
               }
               
               // turn off keep-alive if response has close connection field
               if(keepAlive)
               {
                  if(resHeader->getField("Connection", connHeader) &&
                     strcasecmp(connHeader.c_str(), "close") == 0)
                  {
                     keepAlive = false;
                  }
               }
            }
            else
            {
               // no servicer, so send 404 Not Found
               char html[] = "<html><h2>404 Not Found</h2></html>";
               resHeader->setStatus(404, "Not Found");
               resHeader->setField("Content-Type", "text/html");
               resHeader->setField("Content-Length", 35);
               resHeader->setField("Connection", "close");
               if((noerror = response->sendHeader()))
               {
                  ByteArrayInputStream is(html, 35);
                  noerror = response->sendBody(&is);
               }
            }
         }
         else
         {
            // send 505 HTTP Version Not Supported
            char html[] =
               "<html><h2>505 HTTP Version Not Supported</h2></html>";
            resHeader->setStatus(505, "HTTP Version Not Supported");
            resHeader->setField("Content-Type", "text/html");
            resHeader->setField("Content-Length", 52);
            resHeader->setField("Connection", "close");
            if((noerror = response->sendHeader()))
            {
               ByteArrayInputStream is(html, 52);
               noerror = response->sendBody(&is);
            }
         }
      }
      else
      {
         // exception occurred while receiving header
         ExceptionRef e = Exception::getLast();
         if(!e.isNull() && strcmp(e->getType(), "db.net.http.BadRequest") == 1)
         {
            // send 400 Bad Request
            char html[] = "<html><h2>400 Bad Request</h2></html>";
            response->getHeader()->setStatus(400, "Bad Request");
            response->getHeader()->setField("Content-Type", "text/html");
            response->getHeader()->setField("Content-Length", 38);
            response->getHeader()->setField("Connection", "close");
            if(response->sendHeader())
            {
               ByteArrayInputStream is(html, 38);
               response->sendBody(&is);
            }
         }
         else
         {
            // if the exception was not an interruption or socket error then
            // send an internal server error response
            if(!e.isNull() &&
               strcmp(e->getType(), "db.io.InterruptedException") != 0 &&
               strncmp(e->getType(), "db.net.Socket", 13) != 0)
            {
               // send 500 Internal Server Error
               char html[] = "<html><h2>500 Internal Server Error</h2></html>";
               resHeader->setStatus(500, "Internal Server Error");
               resHeader->setField("Content-Type", "text/html");
               resHeader->setField("Content-Length", 47);
               resHeader->setField("Connection", "close");
               if(response->sendHeader())
               {
                  ByteArrayInputStream is(html, 47);
                  response->sendBody(&is);
               }
            }
         }
      }
      
      if(keepAlive && noerror)
      {
         // clear request and response header fields
         reqHeader->clearFields();
         resHeader->clearFields();
      }
   }
   
   // clean up request and response
   delete request;
   delete response;
   
   // close connection
   hc.close();
}

void HttpConnectionServicer::addRequestServicer(
   HttpRequestServicer* s, bool secure)
{
   mRequestServicerLock.lockExclusive();
   {
      if(secure)
      {
         mSecureServicers[s->getPath()] = s;
      }
      else
      {
         mNonSecureServicers[s->getPath()] = s;
      }
   }
   mRequestServicerLock.unlockExclusive();
}

void HttpConnectionServicer::removeRequestServicer(
   HttpRequestServicer* s, bool secure)
{
   mRequestServicerLock.lockExclusive();
   {
      if(secure)
      {
         mSecureServicers.erase(s->getPath());
      }
      else
      {
         mNonSecureServicers.erase(s->getPath());
      }
   }
   mRequestServicerLock.unlockExclusive();
}

HttpRequestServicer* HttpConnectionServicer::removeRequestServicer(
   const char* path, bool secure)
{
   HttpRequestServicer* rval = NULL;
   
   mRequestServicerLock.lockExclusive();
   {
      if(secure)
      {
         ServicerMap::iterator i = mSecureServicers.find(path);
         if(i != mSecureServicers.end())
         {
            rval = i->second;
            mSecureServicers.erase(path);
         }
      }
      else
      {
         ServicerMap::iterator i = mNonSecureServicers.find(path);
         if(i != mNonSecureServicers.end())
         {
            rval = i->second;
            mNonSecureServicers.erase(path);
         }
      }
   }
   mRequestServicerLock.unlockExclusive();
   
   return rval;
}
