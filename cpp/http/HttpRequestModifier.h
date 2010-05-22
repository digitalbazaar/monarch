/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_http_HttpRequestModifier_H
#define monarch_http_HttpRequestModifier_H

#include "monarch/http/HttpRequest.h"

namespace monarch
{
namespace http
{

/**
 * An HttpRequestModifier is used to modify HttpRequests received over an
 * HttpConnection. For instance, a modifier may rewrite the path in the
 * request header.
 *
 * It implements modifyRequest(HttpRequest* request).
 *
 * @author Dave Longley
 */
class HttpRequestModifier
{
public:
   /**
    * Creates a new HttpRequestModifier.
    */
   HttpRequestModifier();

   /**
    * Destructs this HttpRequestModifier.
    */
   virtual ~HttpRequestModifier();

   /**
    * Modifies the passed HttpRequest. The header for the request has already
    * been received, but the body has not.
    *
    * @param request the HttpRequest to modify.
    */
   virtual void modifyRequest(HttpRequest* request) = 0;
};

} // end namespace http
} // end namespace monarch
#endif
