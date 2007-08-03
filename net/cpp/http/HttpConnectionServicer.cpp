/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpConnectionServicer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "ByteArrayInputStream.h"

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
      path = "/";
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
   string& path, map<string, HttpRequestServicer*>& servicerMap)
{
   HttpRequestServicer* rval = NULL;
   
   lock();
   {
      // try to find servicer at path
      map<string, HttpRequestServicer*>::iterator i = servicerMap.find(path);
      if(i != servicerMap.end())
      {
         rval = servicerMap[path];
      }
      else
      {
         // erase last slash
         path.erase(path.length() - 1, 1);
         while(rval == NULL && path != "")
         {
            // try to find servicer at parent paths
            string::size_type index = path.rfind('/');
            path = path.substr(0, index);
            if(path == "")
            {
               path = "/";
            }
            
            i = servicerMap.find(path);
            if(i != servicerMap.end())
            {
               rval = servicerMap[path];
            }
            else
            {
               // erase last slash
               path.erase(path.length() - 1, 1);
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
   response->getHeader()->setHeader("Server", mServerName);
   
   // receive request header
   IOException* e = request->receiveHeader();
   if(e == NULL)
   {
      // check version
      string version = request->getHeader()->getVersion();
      if(version == "HTTP/1.0" || version == "HTTP/1.1")
      {
         // set version according to request version
         response->getHeader()->setVersion(version);
         
         // include host path if one was used
         string host;
         if(request->getHeader()->getHeader("Host", host))
         {
            response->getHeader()->setHeader("Host", host);
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
            char html[] = "<html>403 Forbidden</html>";
            response->getHeader()->setStatus(403, "Forbidden");
            response->getHeader()->setHeader("Content-Type", "text/html");
            response->getHeader()->setHeader("Content-Length", 26);
            response->getHeader()->setHeader("Connection", "close");
            if(response->sendHeader() == NULL)
            {
               ByteArrayInputStream is(html, 26);
               response->sendBody(&is);
            }
         }
      }
      else
      {
         // send 505 HTTP Version Not Supported
         char html[] = "<html>505 HTTP Version Not Supported</html>";
         response->getHeader()->setStatus(505, "HTTP Version Not Supported");
         response->getHeader()->setHeader("Content-Type", "text/html");
         response->getHeader()->setHeader("Content-Length", 43);
         response->getHeader()->setHeader("Connection", "close");
         if(response->sendHeader() == NULL)
         {
            ByteArrayInputStream is(html, 43);
            response->sendBody(&is);
         }
      }
   }
   else if(strcmp(e->getCode(), "db.net.http.BadRequest") == 1)
   {
      // send 400 Bad Request
      char html[] = "<html>400 Bad Request</html>";
      response->getHeader()->setStatus(400, "Bad Request");
      response->getHeader()->setHeader("Content-Type", "text/html");
      response->getHeader()->setHeader("Content-Length", 29);
      response->getHeader()->setHeader("Connection", "close");
      if(response->sendHeader() == NULL)
      {
         ByteArrayInputStream is(html, 29);
         response->sendBody(&is);
      }
   }
   else
   {
      // if the exception was not an interruption or socket error then
      // send an internal server error response
      Exception* e = Exception::getLast();
      if(e != NULL && dynamic_cast<InterruptedException*>(e) == NULL &&
         dynamic_cast<SocketException*>(e))
      {
         // send 500 Internal Server Error
         char html[] = "<html>500 Internal Server Error</html>";
         response->getHeader()->setStatus(500, "Internal Server Error");
         response->getHeader()->setHeader("Content-Type", "text/html");
         response->getHeader()->setHeader("Content-Length", 38);
         response->getHeader()->setHeader("Connection", "close");
         if(response->sendHeader() == NULL)
         {
            ByteArrayInputStream is(html, 38);
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
   const std::string& path, bool secure)
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
