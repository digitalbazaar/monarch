/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/ws/WebService.h"

#include "monarch/logging/Logging.h"
#include "monarch/util/Timer.h"

#include <cstdio>

using namespace std;
using namespace monarch::http;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;
using namespace monarch::ws;

WebService::WebService(
   const char* path, bool dynamicHandlers, bool pathIsRegex) :
   HttpRequestServicer(path, pathIsRegex),
   mRequestModifier(NULL),
   mDynamicHandlers(dynamicHandlers),
   mAllowHttp1(false)
{
}

WebService::~WebService()
{
   // clean up all paths
   for(HandlerMap::iterator i = mHandlers.begin(); i != mHandlers.end(); ++i)
   {
      free((char*)i->first);
   }
}

void WebService::setRequestModifier(HttpRequestModifier* hrm)
{
   mRequestModifier = hrm;
}

HttpRequestModifier* WebService::getRequestModifier()
{
   return mRequestModifier;
}

void WebService::addHandler(
   const char* path, PathHandlerRef& handler, bool pathIsRegex)
{
   // append path to web service path
   const char* basePath = getPath();
   int length = strlen(basePath) + strlen(path);
   char fullPath[length + 1];
   sprintf(fullPath, "%s%s", basePath, path);

   // normalize full path
   char* normalized = (char*)malloc(length + 2);
   HttpRequestServicer::normalizePath(fullPath, normalized);

   // add full path entry to map
   if(mDynamicHandlers)
   {
      mHandlerLock.lockExclusive();
   }
   mHandlers.insert(make_pair(normalized, handler));
   if(mDynamicHandlers)
   {
      mHandlerLock.unlockExclusive();
   }

   MO_CAT_DEBUG(MO_WS_CAT,
      "Added web service path: %s", (const char*)normalized);
}

PathHandlerRef WebService::removeHandler(const char* path)
{
   PathHandlerRef rval(NULL);

   // append path to web service path
   const char* basePath = getPath();
   int length = strlen(basePath) + strlen(path);
   char fullPath[length + 1];
   sprintf(fullPath, "%s%s", basePath, path);

   // normalize full path
   char normalized[length + 2];
   HttpRequestServicer::normalizePath(fullPath, normalized);

   // erase handler if found
   if(mDynamicHandlers)
   {
      mHandlerLock.lockExclusive();
   }
   HandlerMap::iterator i = mHandlers.find(normalized);
   if(i != mHandlers.end())
   {
      free((char*)i->first);
      rval = i->second;
      mHandlers.erase(i);
   }
   if(mDynamicHandlers)
   {
      mHandlerLock.unlockExclusive();
   }

   if(!rval.isNull())
   {
      MO_CAT_DEBUG(MO_WS_CAT,
         "Removed web service path: %s", (const char*)normalized);
   }

   return rval;
}

void WebService::findHandler(char* path, PathHandlerRef& h)
{
   h.setNull();

   // strip any query
   if(path != NULL)
   {
      char* end = strrchr(path, '?');
      if(end != NULL)
      {
         end[0] = 0;
      }
   }

   if(mDynamicHandlers)
   {
      mHandlerLock.lockShared();
   }

   // try to find handler for path
   HandlerMap::iterator i;
   while(h.isNull() && path != NULL)
   {
      i = mHandlers.find(path);
      if(i != mHandlers.end())
      {
         h = i->second;
      }
      else if(strlen(path) > 1)
      {
         // try to find handler for parent path
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

   if(mDynamicHandlers)
   {
      mHandlerLock.unlockShared();
   }
}

void WebService::serviceRequest(HttpRequest* request, HttpResponse* response)
{
   // print out header
   MO_CAT_DEBUG(MO_WS_CAT,
      "WebService (%s) received header from %s:%i:\n%s",
      request->getConnection()->isSecure() ? "SSL" : "non-SSL",
      request->getConnection()->getRemoteAddress()->getAddress(),
      request->getConnection()->getRemoteAddress()->getPort(),
      request->getHeader()->toString().c_str());

   // set default response server and connection fields
   response->getHeader()->setField("Server", "Monarch WebService/1.0");
   response->getHeader()->setField("Connection", "close");

   // do request modification
   if(mRequestModifier != NULL)
   {
      mRequestModifier->modifyRequest(request);
   }

   // check version (HTTP/1.1 or HTTP/1.0 if allowed)
   if(strcmp(request->getHeader()->getVersion(), "HTTP/1.1") == 0 ||
      (http1Allowed() &&
       strcmp(request->getHeader()->getVersion(), "HTTP/1.0") == 0))
   {
      // start timer
      Timer timer;
      timer.start();

      // set response version
      response->getHeader()->setVersion(request->getHeader()->getVersion());

      // create channel
      PathHandlerRef handler;
      ServiceChannel* channel = createChannel(request, handler);
      channel->initialize();
      channel->setResponse(response);

      // before handler hook
      if(beforePathHandler(channel, handler))
      {
         if(!handler.isNull())
         {
            // handle request
            (*handler)(channel);
         }
         else
         {
            // no handler for path
            response->getHeader()->setStatus(404, "Not Found");
            channel->sendNoContent();
         }
      }

      // print total service time
      MO_CAT_INFO(MO_WS_CAT,
         "WebService (%s) serviced resource '%s %s' for %s:%i "
         "in %" PRIu64 " ms",
         request->getConnection()->isSecure() ? "SSL" : "non-SSL",
         request->getHeader()->getMethod(),
         channel->getPath(),
         request->getConnection()->getRemoteAddress()->getAddress(),
         request->getConnection()->getRemoteAddress()->getPort(),
         timer.getElapsedMilliseconds());

      // clean up channel
      channel->cleanup();
      delete channel;
   }
   else
   {
      // send 505 HTTP Version Not Supported
      response->getHeader()->setVersion("HTTP/1.1");
      response->getHeader()->setStatus(505, "HTTP Version Not Supported");
      response->getHeader()->setField("Content-Length", 0);
      response->sendHeader();

      // print out header
      MO_CAT_DEBUG(MO_WS_CAT,
         "WebService (%s) sent header to %s:%i:\n%s",
         request->getConnection()->isSecure() ? "SSL" : "non-SSL",
         request->getConnection()->getRemoteAddress()->getAddress(),
         request->getConnection()->getRemoteAddress()->getPort(),
         response->getHeader()->toString().c_str());
   }
}

inline void WebService::setAllowHttp1(bool allow)
{
   mAllowHttp1 = allow;
}

inline bool WebService::http1Allowed()
{
   return mAllowHttp1;
}

bool WebService::setKeepAlive(ServiceChannel* ch)
{
   // default to keep-alive for HTTP/1.1 and close for HTTP/1.0
   const char* version = ch->getRequest()->getHeader()->getVersion();
   bool keepAlive = strcmp(version, "HTTP/1.1") == 0;

   string connection;
   if(ch->getRequest()->getHeader()->getField("Connection", connection))
   {
      if(strcasecmp(connection.c_str(), "close") == 0)
      {
         keepAlive = false;
      }
      else if(strcasecmp(connection.c_str(), "keep-alive") == 0)
      {
         keepAlive = true;
      }
   }

   if(keepAlive)
   {
      ch->getResponse()->getHeader()->setField("Connection", "keep-alive");
   }

   return keepAlive;
}

ServiceChannel* WebService::createChannel(
   HttpRequest* request, PathHandlerRef& handler)
{
   ServiceChannel* rval = NULL;

   // normalize request path and find handler
   const char* path = request->getHeader()->getPath();
   char normalized[strlen(path) + 2];
   HttpRequestServicer::normalizePath(path, normalized);
   char copy[strlen(normalized) + 1];
   strcpy(copy, normalized);
   findHandler(normalized, handler);

   // create channel
   rval = new ServiceChannel(copy);

   // set base path
   if(!handler.isNull())
   {
      rval->setBasePath(normalized);
   }

   // set channel request
   rval->setRequest(request);

   return rval;
}

void WebService::setResourceCreated(
   HttpResponse* response, const char* location)
{
   response->getHeader()->setField("Location", location);
   response->getHeader()->setStatus(201, "Created");
}

bool WebService::beforePathHandler(ServiceChannel* ch, PathHandlerRef& handler)
{
   // default hook does nothing
   return true;
}
