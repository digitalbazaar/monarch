/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_WebService_H
#define monarch_ws_WebService_H

#include "monarch/rt/SharedLock.h"
#include "monarch/util/StringTools.h"
#include "monarch/http/HttpRequestModifier.h"
#include "monarch/http/HttpRequestServicer.h"
#include "monarch/ws/PathHandler.h"

namespace monarch
{
namespace ws
{

/**
 * A WebService that provides a set of resources that can be used by a web
 * service client.
 *
 * A client accesses a web service by sending an HTTP request with an
 * appropriate method to one of the service's resources. The service looks
 * for a PathHandler for the path in the HTTP request and uses it to handle
 * the incoming request.
 *
 * @author Dave Longley
 */
class WebService : public monarch::http::HttpRequestServicer
{
public:
   /**
    * Security types for web services.
    */
   enum SecurityType
   {
      Secure,
      NonSecure,
      Both
   };

protected:
   /**
    * A request modifier to use.
    */
   monarch::http::HttpRequestModifier* mRequestModifier;

   /**
    * A map of paths to PathHandlerRefs.
    */
   typedef std::map<
      const char*, PathHandlerRef, monarch::util::StringComparator>
      HandlerMap;
   HandlerMap mHandlers;

   /**
    * A flag to allow dynamic adding/removing of handlers.
    */
   bool mDynamicHandlers;

   /**
    * A lock for manipulating dynamic handlers.
    */
   monarch::rt::SharedLock mHandlerLock;

   /**
    * A flag to allow HTTP/1.0 requests.
    */
   bool mAllowHttp1;

public:
   /**
    * Creates a new WebService that handles requests for the given path or
    * children of that path. The given path will be normalized such that
    * begins with a forward slash and does not end with one.
    *
    * @param path the path this servicer handles requests for.
    * @param dynamicResources true to allow dynamic adding/removing of
    *           resources, false not to.
    */
   WebService(const char* path, bool dynamicResources = false);

   /**
    * Destructs this WebService.
    */
   virtual ~WebService();

   /**
    * Initializes this WebService.
    *
    * @return true if initialized, false if an Exception occurred.
    */
   virtual bool initialize() = 0;

   /**
    * Cleans up this WebService.
    *
    * Must be called after initialize() regardless of its return value.
    */
   virtual void cleanup() = 0;

   /**
    * Sets the request modifier for this service. Should be set before the
    * service starts servicing requests.
    *
    * The given request modifier will be passed incoming HttpRequests after
    * their headers have been read. Changes can be made to the request header
    * such as rewriting the incoming path.
    *
    * @param hrm the HttpRequestModifier to use, NULL for none.
    */
   virtual void setRequestModifier(monarch::http::HttpRequestModifier* hrm);

   /**
    * Gets the request modifier for this service.
    *
    * @return the HttpRequestModifier, NULL for none.
    */
   virtual monarch::http::HttpRequestModifier* getRequestModifier();

   /**
    * Adds a handler to this service. The path will be normalized such that it
    * begins with a forward slash and does not end with one. Consecutive
    * slashes will be normalized to a single slash. It must be relative to the
    * WebService's path.
    *
    * @param path the path to handle.
    * @param handler the path handler.
    */
   virtual void addHandler(const char* path, PathHandlerRef& handler);

   /**
    * Removes a handler from this service. The path will be normalized such
    * that it begins with a forward slash and does not end with one.
    * Consecutive slashes will be normalized to a single slash. It must be
    * relative to the WebService's path.
    *
    * @param path the path for the handler to remove.
    *
    * @return the removed PathHandler, if any (NULL if none).
    */
   virtual PathHandlerRef removeHandler(const char* path);

   /**
    * Gets the PathHandler for the given path, NULL if none exists.
    *
    * @param path the path to get the handler for.
    * @param h the handler reference to update, set to NULL if none exists.
    */
   virtual void findHandler(char* path, PathHandlerRef& h);

   /**
    * Services the passed HttpRequest. The header for the request has already
    * been received, but the body has not. The HttpResponse object is used
    * to send an appropriate response, if necessary, according to the
    * servicer's specific implementation.
    *
    * @param request the HttpRequest to service.
    * @param response the HttpResponse to respond with.
    */
   virtual void serviceRequest(
      monarch::http::HttpRequest* request,
      monarch::http::HttpResponse* response);

   /**
    * Sets whether or not this WebService supports HTTP/1.0 requests.
    *
    * @param allow true to allow HTTP/1.0, false not to.
    */
   virtual void setAllowHttp1(bool allow);

   /**
    * Gets whether or not this WebService supports HTTP/1.0 requests.
    *
    * @return true if HTTP/1.0 allowed, false if not.
    */
   virtual bool http1Allowed();

protected:
   /**
    * Sets the connection to keep-alive if the client supports/requested it.
    *
    * This must be called for each handled path if keep-alive is desired.
    * By default, keep-alive if off for all WebServices.
    *
    * @param ch the ServiceChannel to check/update.
    *
    * @return true if keep-alive is turned on, false if not.
    */
   virtual bool setKeepAlive(ServiceChannel* ch);

   /**
    * Finds the PathHandler for an HttpRequest and creates a ServiceChannel
    * for it. The caller of this method must free the channel.
    *
    * @param request the HttpRequest to create the channel from.
    * @param handler to store the PathHandler for the channel.
    *
    * @return the created ServiceChannel.
    */
   virtual ServiceChannel* createChannel(
      monarch::http::HttpRequest* request, PathHandlerRef& handler);

   /**
    * Sets the location for a created resource and automatically sets the
    * status to 201 "Created."
    *
    * @param response the HttpResponse to respond with.
    * @param location the location of the created resource.
    */
   virtual void setResourceCreated(
      monarch::http::HttpResponse* response, const char* location);

   /**
    * A hook that is called before a PathHandler takes over.
    *
    * @param ch the ServiceChannel.
    * @param handler the PathHandler.
    *
    * @return true to continue to the handler, false to stop handling.
    */
   virtual bool beforePathHandler(ServiceChannel* ch, PathHandlerRef& handler);
};

// type definition for a reference counted WebService
typedef monarch::rt::Collectable<WebService> WebServiceRef;

} // end namespace ws
} // end namespace monarch
#endif
