/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpRequest_H
#define db_net_http_HttpRequest_H

#include "WebRequest.h"
#include "HttpRequestHeader.h"

namespace db
{
namespace net
{
namespace http
{

// forward declare HttpConnection
class HttpConnection;

/**
 * An HttpRequest is a WebRequest that uses the HTTP protocol.
 * 
 * @author Dave Longley
 */
class HttpRequest : public db::net::WebRequest
{
protected:
   /**
    * The header for this request.
    */
   HttpRequestHeader mHeader;
   
public:
   /**
    * Creates a new HttpRequest for the passed HttpConnection.
    * 
    * @param hc the HttpConnection this request is for.
    */
   HttpRequest(HttpConnection* hc);
   
   /**
    * Destructs this HttpRequest.
    */
   virtual ~HttpRequest();
   
   /**
    * Creates a new HttpResponse.
    * 
    * The caller of this method is responsible for freeing the created response.
    * 
    * @return the new HttpResponse.
    */
   virtual db::net::WebResponse* createResponse();
   
   /**
    * Gets the header for this request.
    * 
    * @return the header for this request.
    */
   virtual HttpRequestHeader* getHeader();
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
