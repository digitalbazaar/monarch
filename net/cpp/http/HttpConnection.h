/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpConnection_H
#define db_net_http_HttpConnection_H

#include "WebConnection.h"
#include "InputStream.h"
#include "OutputStream.h"
#include "HttpHeader.h"

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
    * @return an IOException if an IO error occurs, NULL if not.
    */
   virtual db::io::IOException* sendHeader(HttpHeader* header);
   
   /**
    * Receives a message header. This method will block until the entire
    * header has been received, the connection times out, or the thread
    * is interrupted.
    * 
    * @param header the header to populate.
    * 
    * @return an IOException if an IO error occurs, NULL if not.
    */
   virtual db::io::IOException* receiveHeader(HttpHeader* header);
   
   /**
    * Sends the message body for the given header. This method will block
    * until the entire body has been sent, the connection times out, or
    * the thread is interrupted.
    * 
    * @param header the header to send the message body for.
    * @param is the InputStream to read the body from.
    * @param trailers any trailer headers to send if appropriate.
    * 
    * @return an IOException if an IO error occurs, NULL if not.
    */
   virtual db::io::IOException* sendBody(
      HttpHeader* header, db::io::InputStream* is,
      HttpHeader* trailers = NULL);
   
   /**
    * Receives the message body for the given header. This method will block
    * until the entire body has been received, the connection times out, or
    * the thread is interrupted.
    * 
    * @param header the header to receive the message body for.
    * @param os the OutputStream to write the body to.
    * @param trailers used to store any received trailer headers.
    * 
    * @return an IOException if an IO error occurs, NULL if not.
    */
   virtual db::io::IOException* receiveBody(
      HttpHeader* header, db::io::OutputStream* os,
      HttpHeader* trailers = NULL);
   
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
