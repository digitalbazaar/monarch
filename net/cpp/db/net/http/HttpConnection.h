/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpConnection_H
#define db_net_http_HttpConnection_H

#include "db/net/WebConnection.h"
#include "db/io/InputStream.h"
#include "db/io/OutputStream.h"
#include "db/net/http/HttpTrailer.h"

namespace db
{
namespace net
{
namespace http
{

/**
 * An HttpConnection is a WebConnection that uses the HTTP protocol.
 * 
 * @author Dave Longley
 */
class HttpConnection : public db::net::WebConnection
{
protected:
   /**
    * The total number of content bytes read by this HttpConnection.
    */
   unsigned long long mContentBytesRead;
   
   /**
    * The total number of content bytes written by this HttpConnection.
    */
   unsigned long long mContentBytesWritten;
   
   /**
    * HttpOutputStream is a friend so it can access the underlying
    * ConnectionOutputStream.
    */
   friend class HttpBodyOutputStream;
   
public:
   /**
    * Creates a new HttpConnection that wraps the passed Connection.
    * 
    * @param c the Connection to wrap.
    * @param cleanup true to clean up the Connection when this HttpConnection
    *                is destructed, false to leave it alone. 
    */
   HttpConnection(Connection* c, bool cleanup);
   
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
   virtual db::net::WebRequest* createRequest();
   
   /**
    * Sends a message header. This method will block until the entire header
    * has been sent, the connection times out, or the thread is interrupted.
    * 
    * @param header the header to send.
    * 
    * @return true if the header was sent, false if an IOException occurred.
    */
   virtual bool sendHeader(HttpHeader* header);
   
   /**
    * Receives a message header. This method will block until the entire
    * header has been received, the connection times out, or the thread
    * is interrupted.
    * 
    * @param header the header to populate.
    * 
    * @return true if the header was received, false if an IOException occurred.
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
    * @return true if the body was sent, false if an IOException occurred.
    */
   virtual bool sendBody(
      HttpHeader* header, db::io::InputStream* is,
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
   virtual db::io::OutputStream* getBodyOutputStream(
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
    * @return true if the body was received, false if an IOException occurred.
    */
   virtual bool receiveBody(
      HttpHeader* header, db::io::OutputStream* os,
      HttpTrailer* trailer = NULL);
   
   /**
    * Sets the total number of content bytes read from this HttpConnection so
    * far. This includes any bytes that were skipped but not any bytes that
    * were peeked.
    * 
    * @param count the total number of content bytes read so far.
    */
   virtual void setContentBytesRead(unsigned long long count);
   
   /**
    * Gets the total number of content bytes read from this HttpConnection so
    * far. This includes any bytes that were skipped but not any bytes that
    * were peeked.
    * 
    * @return the total number of content bytes read so far.
    */
   virtual unsigned long long getContentBytesRead();
   
   /**
    * Sets the number of content bytes written to this HttpConnection so far.
    * 
    * @param count the number of content bytes written so far.
    */
   virtual void setContentBytesWritten(unsigned long long count);
   
   /**
    * Gets the number of content bytes written to this HttpConnection so far.
    * 
    * @return the number of content bytes written so far.
    */
   virtual unsigned long long getContentBytesWritten();
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
