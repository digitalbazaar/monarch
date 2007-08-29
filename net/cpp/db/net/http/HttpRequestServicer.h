/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpRequestServicer_H
#define db_net_http_HttpRequestServicer_H

#include "db/net/http/HttpRequest.h"
#include "db/net/http/HttpResponse.h"

namespace db
{
namespace net
{
namespace http
{

/**
 * An HttpRequestServicer services HttpRequests received over an HttpConnection.
 * 
 * It implements serviceRequest(HttpRequest*, HttpResponse*) and uses the
 * passed objects to communicate.
 * 
 * @author Dave Longley
 */
class HttpRequestServicer
{
protected:
   /**
    * The path for this servicer.
    */
   char* mPath;
   
public:
   /**
    * Creates a new HttpRequestServicer that handles requests for the
    * given path or children of that path. If the given path doesn't
    * begin and end with forward slashes, they will be prepended/appended.
    * 
    * @param path the path this servicer handles requests for.
    */
   HttpRequestServicer(const char* path);
   
   /**
    * Destructs this HttpRequestServicer.
    */
   virtual ~HttpRequestServicer();
   
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
      HttpRequest* request, HttpResponse* response) = 0;
   
   /**
    * Returns the path this servicer handles requests for.
    * 
    * @return the path this servicer handles requests for.
    */
   virtual const char* getPath();
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
