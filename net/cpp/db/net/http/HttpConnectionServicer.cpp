/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpConnectionServicer.h"

#include "db/net/http/HttpRequest.h"
#include "db/net/http/HttpResponse.h"
#include "db/io/ByteArrayInputStream.h"

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
   
   lock();
   {
      // strip any query
      if(path != NULL)
      {
         char* end = strrchr(path, '?');
         if(end != NULL)
         {
            end[0] = 0;
         }
      }
      
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
   unlock();
   
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
   
   // create response, set defaults
   HttpResponse* response = (HttpResponse*)request->createResponse();
   response->getHeader()->setVersion("HTTP/1.1");
   response->getHeader()->setDate();
   response->getHeader()->setField("Server", mServerName);
   
   // receive request header
   if(request->receiveHeader())
   {
      // check version
      if(strcmp(request->getHeader()->getVersion(), "HTTP/1.0") == 0 ||
         strcmp(request->getHeader()->getVersion(), "HTTP/1.1") == 0)
      {
         // set version according to request version
         response->getHeader()->setVersion(request->getHeader()->getVersion());
         
         // include host path if one was used
         string host;
         if(request->getHeader()->getField("Host", host))
         {
            response->getHeader()->setField("Host", host);
         }
         
         // get request path and normalize it
         const char* inPath = request->getHeader()->getPath();
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
         }
         else
         {
            // no servicer, so send 404 Not Found
            char html[] = "<html><h2>404 Not Found</h2></html>";
            response->getHeader()->setStatus(404, "Not Found");
            response->getHeader()->setField("Content-Type", "text/html");
            response->getHeader()->setField("Content-Length", 35);
            response->getHeader()->setField("Connection", "close");
            if(response->sendHeader())
            {
               ByteArrayInputStream is(html, 35);
               response->sendBody(&is);
            }
         }
      }
      else
      {
         // send 505 HTTP Version Not Supported
         char html[] = "<html><h2>505 HTTP Version Not Supported</h2></html>";
         response->getHeader()->setStatus(505, "HTTP Version Not Supported");
         response->getHeader()->setField("Content-Type", "text/html");
         response->getHeader()->setField("Content-Length", 52);
         response->getHeader()->setField("Connection", "close");
         if(response->sendHeader())
         {
            ByteArrayInputStream is(html, 52);
            response->sendBody(&is);
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
            dynamic_cast<InterruptedException*>(&(*e)) == NULL &&
            dynamic_cast<SocketException*>(&(*e)) == NULL)
         {
            // send 500 Internal Server Error
            char html[] = "<html><h2>500 Internal Server Error</h2></html>";
            response->getHeader()->setStatus(500, "Internal Server Error");
            response->getHeader()->setField("Content-Type", "text/html");
            response->getHeader()->setField("Content-Length", 47);
            response->getHeader()->setField("Connection", "close");
            if(response->sendHeader())
            {
               ByteArrayInputStream is(html, 47);
               response->sendBody(&is);
            }
         }
      }
   }
   
   // clean up request and response
   delete request;
   delete response;
}

void HttpConnectionServicer::addRequestServicer(
   HttpRequestServicer* s, bool secure)
{
   lock();
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
   unlock();
}

void HttpConnectionServicer::removeRequestServicer(
   HttpRequestServicer* s, bool secure)
{
   lock();
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
   unlock();
}

HttpRequestServicer* HttpConnectionServicer::removeRequestServicer(
   const char* path, bool secure)
{
   HttpRequestServicer* rval = NULL;
   
   lock();
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
   unlock();
   
   return rval;
}
