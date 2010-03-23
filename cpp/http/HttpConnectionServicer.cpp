/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpConnectionServicer.h"

#include "monarch/http/HttpRequest.h"
#include "monarch/http/HttpResponse.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/logging/Logging.h"
#include "monarch/rt/DynamicObjectIterator.h"

#include <cstdlib>

using namespace std;
using namespace monarch::io;
using namespace monarch::http;
using namespace monarch::logging;
using namespace monarch::net;
using namespace monarch::rt;

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
               // if parent is root (end == path), set path to "/"
               // if parent is not root, clear last slash
               end[(end == path) ? 1 : 0] = 0;
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
   HttpRequest* request = hc.createRequest();
   HttpRequestHeader* reqHeader = request->getHeader();

   // create response
   HttpResponse* response = request->createResponse();
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

            // use proxy'd host field if one was used
            // else use host field if one was used
            string host;
            if(reqHeader->getField("X-Forwarded-Host", host) ||
               reqHeader->getField("Host", host))
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
               const char* html =
                  "<html><body><h2>404 Not Found</h2></body></html>";
               resHeader->setStatus(404, "Not Found");
               resHeader->setField("Content-Type", "text/html");
               resHeader->setField("Content-Length", 48);
               resHeader->setField("Connection", "close");
               if((noerror = response->sendHeader()))
               {
                  ByteArrayInputStream is(html, 48);
                  noerror = response->sendBody(&is);
               }
            }
         }
         else
         {
            // send 505 HTTP Version Not Supported
            const char* html =
               "<html><body>"
               "<h2>505 HTTP Version Not Supported</h2>"
               "</body></html>";
            resHeader->setStatus(505, "HTTP Version Not Supported");
            resHeader->setField("Content-Type", "text/html");
            resHeader->setField("Content-Length", 65);
            resHeader->setField("Connection", "close");
            if((noerror = response->sendHeader()))
            {
               ByteArrayInputStream is(html, 65);
               noerror = response->sendBody(&is);
            }
         }
      }
      else
      {
         // exception occurred while receiving header
         ExceptionRef e = Exception::get();
         if(e->isType("monarch.net.http.BadHeader") ||
            e->isType("monarch.net.http.BadRequest"))
         {
            // send 400 Bad Request
            const char* html =
               "<html><body><h2>400 Bad Request</h2></body></html>";
            response->getHeader()->setStatus(400, "Bad Request");
            response->getHeader()->setField("Content-Type", "text/html");
            response->getHeader()->setField("Content-Length", 50);
            response->getHeader()->setField("Connection", "close");
            if(response->sendHeader())
            {
               ByteArrayInputStream is(html, 50);
               response->sendBody(&is);
            }
         }
         // if the exception was an interruption, then send a 503
         else if(e->isType("monarch.io.InterruptedException") ||
                 e->isType("monarch.rt.Interrupted"))
         {
            // send 503 Service Unavailable
            const char* html =
               "<html><body><h2>503 Service Unavailable</h2></body></html>";
            resHeader->setStatus(503, "Service Unavailable");
            resHeader->setField("Content-Type", "text/html");
            resHeader->setField("Content-Length", 58);
            resHeader->setField("Connection", "close");
            if((noerror = response->sendHeader()))
            {
               ByteArrayInputStream is(html, 58);
               noerror = response->sendBody(&is);
            }
         }
         // if the exception was not a socket error then send an internal
         // server error response
         else if(!e->isType("monarch.net.Socket", true))
         {
            // send 500 Internal Server Error
            const char* html =
               "<html><body><h2>500 Internal Server Error</h2></body></html>";
            resHeader->setStatus(500, "Internal Server Error");
            resHeader->setField("Content-Type", "text/html");
            resHeader->setField("Content-Length", 60);
            resHeader->setField("Connection", "close");
            if((noerror = response->sendHeader()))
            {
               ByteArrayInputStream is(html, 60);
               noerror = response->sendBody(&is);
            }
         }
         else
         {
            // log socket error
            if(e->getDetails()->hasMember("error"))
            {
               // build error string
               string error;
               DynamicObjectIterator i =
                  e->getDetails()["error"].getIterator();
               while(i->hasNext())
               {
                  error.append(i->next()->getString());
                  if(i->hasNext())
                  {
                     error.push_back(',');
                  }
               }
               MO_CAT_ERROR(MO_HTTP_CAT,
                  "Connection error: ['%s','%s','%s']\n",
                  e->getMessage(), e->getType(), error.c_str());
            }
            else
            {
               MO_CAT_ERROR(MO_HTTP_CAT,
                  "Connection error: ['%s','%s']\n",
                  e->getMessage(), e->getType());
            }
         }
      }

      if(keepAlive && noerror)
      {
         // set keep-alive timeout (defaults to 5 minutes)
         hc.setReadTimeout(1000 * 60 * 5);

         // clear request and response header fields
         reqHeader->clearFields();
         resHeader->clearFields();
         resHeader->clearStatus();
      }
   }

   // clean up request and response
   delete request;
   delete response;

   // close connection
   hc.close();
}

bool HttpConnectionServicer::addRequestServicer(
   HttpRequestServicer* s, bool secure)
{
   bool rval = false;

   const char* path = s->getPath();

   mRequestServicerLock.lockExclusive();
   {
      if(secure)
      {
         ServicerMap::iterator i = mSecureServicers.find(path);
         if(i == mSecureServicers.end())
         {
            mSecureServicers[path] = s;
            rval = true;
         }
      }
      else
      {
         ServicerMap::iterator i = mNonSecureServicers.find(path);
         if(i == mNonSecureServicers.end())
         {
            mNonSecureServicers[path] = s;
            rval = true;
         }
      }
   }
   mRequestServicerLock.unlockExclusive();

   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Could not add http request servicer. Path already in use.",
         "monarch.net.http.DuplicatePath");
      e->getDetails()["path"] = path;
      Exception::set(e);
   }

   return rval;
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
            mSecureServicers.erase(i);
         }
      }
      else
      {
         ServicerMap::iterator i = mNonSecureServicers.find(path);
         if(i != mNonSecureServicers.end())
         {
            rval = i->second;
            mNonSecureServicers.erase(i);
         }
      }
   }
   mRequestServicerLock.unlockExclusive();

   return rval;
}

HttpRequestServicer* HttpConnectionServicer::getRequestServicer(
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
         }
      }
      else
      {
         ServicerMap::iterator i = mNonSecureServicers.find(path);
         if(i != mNonSecureServicers.end())
         {
            rval = i->second;
         }
      }
   }
   mRequestServicerLock.unlockExclusive();

   return rval;
}
