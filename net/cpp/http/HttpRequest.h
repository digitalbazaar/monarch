/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef HttpRequest_H
#define HttpRequest_H

#include "WebRequest.h"
#include "HttpHeader.h"

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
    * The HttpHeader for this request.
    */
   HttpHeader mHeader;
   
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
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
