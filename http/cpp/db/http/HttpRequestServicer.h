/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_http_HttpRequestServicer_H
#define db_http_HttpRequestServicer_H

#include "db/http/HttpRequest.h"
#include "db/http/HttpResponse.h"

namespace db
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
    * begin with a forward slash and not end with one, then it will
    * be normalized such that it does. Consecutive slashes will be
    * normalized to a single slash. A path "servicer//path/" will
    * be transformed into: "/servicer/path".
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
   
   /**
    * Normalizes "inPath" to "outPath" by prepending a forward slash if
    * necessary and by ensuring the path does not end in a forward slash.
    *  
    * @param inPath the path to normalize.
    * @param outPath the string to store the normalized path in, which must
    *                be at least [strlen(inPath) + 2] in size.
    */
   static void normalizePath(const char* inPath, char* outPath);
};

} // end namespace http
} // end namespace db
#endif
