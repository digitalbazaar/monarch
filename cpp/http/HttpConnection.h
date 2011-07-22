/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_http_HttpConnection_H
#define monarch_http_HttpConnection_H

#include "monarch/net/ConnectionWrapper.h"
#include "monarch/http/HttpRequest.h"
#include "monarch/http/HttpRequestState.h"
#include "monarch/http/HttpTrailer.h"

namespace monarch
{
namespace http
{

/**
 * An HttpConnection is a Connection that uses the HTTP protocol.
 *
 * @author Dave Longley
 */
class HttpConnection : public monarch::net::ConnectionWrapper
{
protected:
   /**
    * The total number of content bytes read by this HttpConnection.
    */
   uint64_t mContentBytesRead;

   /**
    * The total number of content bytes written by this HttpConnection.
    */
   uint64_t mContentBytesWritten;

   /**
    * A buffer for reading/writing.
    */
   monarch::io::ByteBuffer mBuffer;

   /**
    * Container for request state.
    */
   HttpRequestState* mRequestState;

public:
   /**
    * Creates a new HttpConnection that wraps the passed Connection.
    *
    * @param c the Connection to wrap.
    * @param cleanup true to clean up the Connection when this HttpConnection
    *                is destructed, false to leave it alone.
    */
   HttpConnection(monarch::net::Connection* c, bool cleanup);

   /**
    * Destructs this HttpConnection.
    */
   virtual ~HttpConnection();

   /**
    * Creates a new HttpRequest.
    *
    * The caller of this method is responsible for freeing the created request.
    *
    * @return the new HttpRequest.
    */
   virtual HttpRequest* createRequest();

   /**
    * Sends a message header. This method will block until the entire header
    * has been sent, the connection times out, or the thread is interrupted.
    *
    * @param header the header to send.
    *
    * @return true if the header was sent, false if an Exception occurred.
    */
   virtual bool sendHeader(HttpHeader* header);

   /**
    * Receives a message header. This method will block until the entire
    * header has been received, the connection times out, or the thread
    * is interrupted.
    *
    * @param header the header to populate.
    *
    * @return true if the header was received, false if an Exception occurred.
    */
   virtual bool receiveHeader(HttpHeader* header);

   /**
    * Sends the message body for the given header. This method will block
    * until the entire body has been sent, the connection times out, or
    * the thread is interrupted.
    *
    * @param header the header to send the message body for.
    * @param is the InputStream to read the body from.
    * @param trailer any trailer headers to send if appropriate.
    *
    * @return true if the body was sent, false if an Exception occurred.
    */
   virtual bool sendBody(
      HttpHeader* header, monarch::io::InputStream* is,
      HttpTrailer* trailer = NULL);

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
   virtual monarch::io::OutputStream* getBodyOutputStream(
      HttpHeader* header, HttpTrailer* trailer = NULL);

   /**
    * Receives the message body for the given header. This method will block
    * until the entire body has been received, the connection times out, or
    * the thread is interrupted.
    *
    * @param header the header to receive the message body for.
    * @param os the OutputStream to write the body to.
    * @param trailer used to store any received trailer headers.
    *
    * @return true if the body was received, false if an Exception occurred.
    */
   virtual bool receiveBody(
      HttpHeader* header, monarch::io::OutputStream* os,
      HttpTrailer* trailer = NULL);

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
   virtual monarch::io::InputStream* getBodyInputStream(
      HttpHeader* header, HttpTrailer* trailer = NULL);

   /**
    * Sets the total number of content bytes read from this HttpConnection so
    * far. This includes any bytes that were skipped but not any bytes that
    * were peeked.
    *
    * @param count the total number of content bytes read so far.
    */
   virtual void setContentBytesRead(uint64_t count);

   /**
    * Gets the total number of content bytes read from this HttpConnection so
    * far. This includes any bytes that were skipped but not any bytes that
    * were peeked.
    *
    * @return the total number of content bytes read so far.
    */
   virtual uint64_t getContentBytesRead();

   /**
    * Sets the number of content bytes written to this HttpConnection so far.
    *
    * @param count the number of content bytes written so far.
    */
   virtual void setContentBytesWritten(uint64_t count);

   /**
    * Gets the number of content bytes written to this HttpConnection so far.
    *
    * @return the number of content bytes written so far.
    */
   virtual uint64_t getContentBytesWritten();

   /**
    * Sets the request state object. This object will be cleaned up.
    *
    * @param state a HttpRequestState object.
    */
   virtual void setRequestState(HttpRequestState* state);

   /**
    * Gets the request state object. If one has not been set a
    * HttpRequestState will be created.
    *
    * @return the request state.
    */
   virtual HttpRequestState* getRequestState();
};

// typedef for a counted reference to an HttpConnection
typedef monarch::rt::Collectable<HttpConnection> HttpConnectionRef;

} // end namespace http
} // end namespace monarch
#endif
