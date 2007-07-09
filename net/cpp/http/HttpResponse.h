/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpResponse_H
#define db_net_http_HttpResponse_H

#include "WebResponse.h"
#include "HttpResponseHeader.h"

namespace db
{
namespace net
{
namespace http
{

// forward declare HttpRequest
class HttpRequest;

/**
 * An HttpResponse is a WebResponse that uses the HTTP protocol.
 * 
 * @author Dave Longley
 */
class HttpResponse : public db::net::WebResponse
{
protected:
   /**
    * The header for this response.
    */
   HttpResponseHeader mHeader;
   
public:
   /**
    * Creates a new HttpResponse for the passed HttpRequest.
    * 
    * @param request the HttpRequest this response is for.
    */
   HttpResponse(HttpRequest* request);
   
   /**
    * Destructs this HttpResponse.
    */
   virtual ~HttpResponse();
   
   /**
    * Gets the header for this response.
    * 
    * @return the header for this response.
    */
   virtual HttpResponseHeader* getHeader();
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
