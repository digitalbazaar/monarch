/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_http_HttpBodyInputStream_H
#define db_http_HttpBodyInputStream_H

#include "monarch/io/FilterInputStream.h"
#include "monarch/http/HttpConnection.h"
#include "monarch/http/HttpTrailer.h"

namespace db
{
namespace http
{

/**
 * A HttpBodyInputStream is a class that is used to receive an http body. It
 * will use the appropriate transfer-encoding (i.e. "chunked") if the
 * provided HttpHeader calls for it. Closing the stream will not shut down
 * input or close the http connection.
 *
 * @author Dave Longley
 */
class HttpBodyInputStream : public db::io::FilterInputStream
{
protected:
   /**
    * The associated HttpConnection.
    */
   HttpConnection* mConnection;

   /**
    * The associated HttpTrailer.
    */
   HttpTrailer* mTrailer;

   /**
    * Stores the amount of data received in the body.
    */
   int64_t mBytesReceived;

   /**
    * True if using chunked transfer encoding, false if not.
    */
   bool mChunkedTransfer;

   /**
    * Stores the content length.
    */
   int64_t mContentLength;

   /**
    * Stores whether or not the content length is known.
    */
   bool mContentLengthKnown;

public:
   /**
    * Creates a new HttpBodyInputStream.
    *
    * @param hc the HttpConnection to receive data over.
    * @param header the HttpHeader for the body.
    * @param trailer the HttpTrailer to use for storing header trailers.
    */
   HttpBodyInputStream(
      HttpConnection* hc, HttpHeader* header, HttpTrailer* trailer);

   /**
    * Destructs this HttpBodyInputStream.
    */
   virtual ~HttpBodyInputStream();

   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of 0 will be returned if the end of the stream has been reached,
    * a value of -1 will be returned if an IO exception occurred, otherwise
    * the number of bytes read will be returned.
    *
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    *
    * @return the number of bytes read from the stream or 0 if the end of the
    *         stream has been reached or -1 if an IO exception occurred.
    */
   virtual int read(char* b, int length);

   /**
    * Closes the stream. This will not close the underlying http stream.
    */
   virtual void close();
};

} // end namespace http
} // end namespace db
#endif
