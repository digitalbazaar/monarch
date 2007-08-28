/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpRequest_H
#define db_net_http_HttpRequest_H

#include "WebRequest.h"
#include "HttpRequestHeader.h"
#include "HttpTrailer.h"

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
    * Sends the header for this request. This method will block until the
    * entire header has been sent, the connection times out, or the thread
    * is interrupted.
    * 
    * @return an IOException if an IO error occurs, NULL if not.
    */
   virtual db::io::IOException* sendHeader();
   
   /**
    * Receives the header for this request. This method will block until the
    * entire header has been received, the connection times out, or the thread
    * is interrupted.
    * 
    * @return an IOException if an IO error occurs, NULL if not.
    */
   virtual db::io::IOException* receiveHeader();
   
   /**
    * Sends the body for this request. This method will block until the
    * entire body has been sent, the connection times out, or the thread is
    * interrupted.
    * 
    * @param is the InputStream to read the body from.
    * @param trailer header trailers to send.
    * 
    * @return an IOException if an IO error occurs, NULL if not.
    */
   virtual db::io::IOException* sendBody(
      db::io::InputStream* is, HttpTrailer* trailer = NULL);
   
   /**
    * Receives the body for this request. This method will block until the
    * entire body has been received, the connection times out, or the thread
    * is interrupted.
    * 
    * @param os the OutputStream to write the body to.
    * @param trailer used to store received header trailers.
    * 
    * @return an IOException if an IO error occurs, NULL if not.
    */
   virtual db::io::IOException* receiveBody(
      db::io::OutputStream* os, HttpTrailer* trailer = NULL);
   
   /**
    * Gets the header for this request. This will not receive the header
    * from the underlying connection, it will only return the header object.
    * 
    * @return the header for this request.
    */
   virtual HttpRequestHeader* getHeader();
   
   /**
    * Gets the HttpConnection associated with this request.
    * 
    * @return the HttpConnection associated with this request.
    */
   virtual HttpConnection* getConnection();
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
