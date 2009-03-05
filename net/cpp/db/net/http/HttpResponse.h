/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpResponse_H
#define db_net_http_HttpResponse_H

#include "db/net/http/HttpResponseHeader.h"
#include "db/net/http/HttpConnection.h"

namespace db
{
namespace net
{
namespace http
{

// forward declare classes
class HttpRequest;

/**
 * An HttpResponse is a web response that uses the HTTP protocol.
 * 
 * @author Dave Longley
 */
class HttpResponse
{
protected:
   /**
    * The header for this response.
    */
   HttpResponseHeader mHeader;
   
   /**
    * The HttpRequest this response is for.
    */
   HttpRequest* mRequest;
   
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
    * @return true if the header was sent, false if an Exception occurred.
    */
   virtual bool sendHeader();
   
   /**
    * Receives the header for this response. This method will block until the
    * entire header has been received, the connection times out, or the thread
    * is interrupted.
    * 
    * @return true if the header was received, false if an Exception occurred.
    */
   virtual bool receiveHeader();
   
   /**
    * Sends the body for this response. This method will block until the
    * entire body has been sent, the connection times out, or the thread is
    * interrupted.
    * 
    * @param is the InputStream to read the body from.
    * @param trailer header trailers to send.
    * 
    * @return true if the body was sent, false if an Exception occurred.
    */
   virtual bool sendBody(db::io::InputStream* is, HttpTrailer* trailer = NULL);
   
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
    * @return the heap-allocated OutputStream for sending a message body.
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
    * @return true if the body was received, false if an Exception occurred.
    */
   virtual bool receiveBody(
      db::io::OutputStream* os, HttpTrailer* trailer = NULL);
   
   /**
    * Gets a heap-allocated InputStream for receiving a message body. The
    * stream must be closed and deleted when it is finished being used. Closing
    * the stream will not shut down input or close the connection. The stream
    * will automatically handle transfer-encoding (i.e. "chunked") based on
    * the passed header. The stream will also automatically update the number
    * of content bytes received by this connection.
    * 
    * @param header the header to receive the message body for.
    * @param trailer user to store any received trailer headers.
    * 
    * @return the heap-allocated InputStream for receiving a message body.
    */
   virtual db::io::InputStream* getBodyInputStream(HttpTrailer* trailer = NULL);
   
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
   
   /**
    * Gets the related request.
    * 
    * @return the related http request.
    */
   virtual HttpRequest* getRequest();
};

// typedef for a counted reference to an HttpResponse
typedef db::rt::Collectable<HttpResponse> HttpResponseRef;

} // end namespace http
} // end namespace net
} // end namespace db
#endif
