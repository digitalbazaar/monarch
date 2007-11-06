/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpResponse_H
#define db_net_http_HttpResponse_H

#include "db/net/WebResponse.h"
#include "db/net/http/HttpResponseHeader.h"
#include "db/net/http/HttpConnection.h"

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
    * Sends the header for this response. This method will block until the
    * entire header has been sent, the connection times out, or the thread
    * is interrupted.
    * 
    * @return an IOException if an IO error occurs, NULL if not.
    */
   virtual db::io::IOException* sendHeader();
   
   /**
    * Receives the header for this response. This method will block until the
    * entire header has been received, the connection times out, or the thread
    * is interrupted.
    * 
    * @return an IOException if an IO error occurs, NULL if not.
    */
   virtual db::io::IOException* receiveHeader();
   
   /**
    * Sends the body for this response. This method will block until the
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
    * Gets a heap-allocated OutputStream for sending a message body. The
    * stream must be closed and deleted when it is finished being used. Closing
    * the stream will not shut down output or close the connection. The stream
    * will automatically handle transfer-encoding (i.e. "chunked") based on
    * the passed header. The stream will also automatically update the number
    * of content bytes sent by this connection. The stream will not ensure that
    * the number of content-bytes sent matches the Content-Length (if one
    * was specified).
    * 
    * @param header the header to send the message body for.
    * @param trailer any trailer headers to send if appropriate.
    * 
    * @return an IOException if an IO error occurs, NULL if not.
    */
   virtual db::io::OutputStream* getBodyOutputStream(
      HttpTrailer* trailer = NULL);
   
   /**
    * Receives the body for this response. This method will block until the
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
    * Gets the header for this response. This will not receive the header
    * from the underlying connection, it will only return the header object.
    * 
    * @return the header for this response.
    */
   virtual HttpResponseHeader* getHeader();
   
   /**
    * Gets the HttpConnection associated with this response.
    * 
    * @return the HttpConnection associated with this response.
    */
   virtual HttpConnection* getConnection();
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
