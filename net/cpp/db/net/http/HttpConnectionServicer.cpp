/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpConnectionServicer.h"
#include "db/net/http/HttpRequest.h"
#include "db/net/http/HttpResponse.h"
#include "db/io/ByteArrayInputStream.h"

using namespace std;
using namespace db::io;
using namespace db::net::http;
using namespace db::rt;

HttpConnectionServicer::HttpConnectionServicer(const string& serverName)
{
   mServerName = serverName;
}

HttpConnectionServicer::~HttpConnectionServicer()
{
}

void HttpConnectionServicer::normalizePath(string& path)
{
   if(path.length() == 0)
   {
      path.append(1, '/');
   }
   else
   {
      // prepend slash as necessary
      if(path[0] != '/')
      {
         path.insert(0, 1, '/');
      }
      
      // append slash as necessary
      if(path[path.length() - 1] != '/')
      {
         path.append(1, '/');
      }
   }
}

HttpRequestServicer* HttpConnectionServicer::findRequestServicer(
   string& path,
   map<const char*, HttpRequestServicer*, PathComparator>& servicerMap)
{
   HttpRequestServicer* rval = NULL;
   
   lock();
   {
      // try to find servicer at path
      map<const char*, HttpRequestServicer*, PathComparator>::iterator i =
         servicerMap.find(path.c_str());
      if(i != servicerMap.end())
      {
         rval = i->second;
      }
      else
      {
         while(rval == NULL && path.length() > 1)
         {
            // try to find servicer at parent paths
            string::size_type index = path.rfind('/', path.length() - 2);
            path = path.substr(0, index + 1);
            
            i = servicerMap.find(path.c_str());
            if(i != servicerMap.end())
            {
               rval = i->second;
            }
         }
      }
   }
   unlock();
   
   return rval;
}

void HttpConnectionServicer::serviceConnection(Connection* c)
{
   // wrap connection, set default read timeout to 30 seconds
   HttpConnection hc(c, false);
   hc.setReadTimeout(30000);
   
   // create request
   HttpRequest* request = (HttpRequest*)hc.createRequest();
   
   // create response, set defaults
   HttpResponse* response = (HttpResponse*)request->createResponse();
   response->getHeader()->setVersion("HTTP/1.1");
   response->getHeader()->setDate();
   response->getHeader()->setField("Server", mServerName);
   
   // receive request header
   IOException* e = request->receiveHeader();
   if(e == NULL)
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
         string path = request->getHeader()->getPath();
         normalizePath(path);
         
         // find appropriate request servicer for path
         HttpRequestServicer* hrs = NULL;
         
         if(hc.isSecure())
         {
            // find secure servicer
            hrs = findRequestServicer(path, mSecureServicers);
         }
         else
         {
            // find non-secure servicer
            hrs = findRequestServicer(path, mNonSecureServicers);
         }
         
         if(hrs != NULL)
         {
            // service request
            hrs->serviceRequest(request, response);
         }
         else
         {
            // no servicer, so send 403 Forbidden
            char html[] = "<html><h2>403 Forbidden</h2></html>";
            response->getHeader()->setStatus(403, "Forbidden");
            response->getHeader()->setField("Content-Type", "text/html");
            response->getHeader()->setField("Content-Length", 35);
            response->getHeader()->setField("Connection", "close");
            if(response->sendHeader() == NULL)
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
         if(response->sendHeader() == NULL)
         {
            ByteArrayInputStream is(html, 52);
            response->sendBody(&is);
         }
      }
   }
   else if(strcmp(e->getCode(), "db.net.http.BadRequest") == 1)
   {
      // send 400 Bad Request
      char html[] = "<html><h2>400 Bad Request</h2></html>";
      response->getHeader()->setStatus(400, "Bad Request");
      response->getHeader()->setField("Content-Type", "text/html");
      response->getHeader()->setField("Content-Length", 38);
      response->getHeader()->setField("Connection", "close");
      if(response->sendHeader() == NULL)
      {
         ByteArrayInputStream is(html, 38);
         response->sendBody(&is);
      }
   }
   else
   {
      // if the exception was not an interruption or socket error then
      // send an internal server error response
      Exception* e = Exception::getLast();
      if(e != NULL && dynamic_cast<InterruptedException*>(e) == NULL &&
         dynamic_cast<SocketException*>(e) == NULL)
      {
         // send 500 Internal Server Error
         char html[] = "<html><h2>500 Internal Server Error</h2></html>";
         response->getHeader()->setStatus(500, "Internal Server Error");
         response->getHeader()->setField("Content-Type", "text/html");
         response->getHeader()->setField("Content-Length", 47);
         response->getHeader()->setField("Connection", "close");
         if(response->sendHeader() == NULL)
         {
            ByteArrayInputStream is(html, 47);
            response->sendBody(&is);
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

void HttpConnectionServicer::removeRequestServicer(
   const char* path, bool secure)
{
   lock();
   {
      if(secure)
      {
         mSecureServicers.erase(path);
      }
      else
      {
         mNonSecureServicers.erase(path);
      }
   }
   unlock();
}
