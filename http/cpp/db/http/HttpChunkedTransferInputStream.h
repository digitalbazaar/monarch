/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_http_HttpChunkedTransferInputStream_H
#define db_http_HttpChunkedTransferInputStream_H

#include "db/io/PeekInputStream.h"
#include "db/net/ConnectionInputStream.h"
#include "db/http/HttpTrailer.h"
#include "db/rt/Thread.h"

namespace db
{
namespace http
{

/**
 * A HttpChunkedTransferInputStream is a class that is used to decode
 * http message bodies for http requests and and http responses that have a
 * transfer-encoding header value set to "chunked".
 *
 * Chunked Transfer Coding breaks an http message into a series of chunks,
 * each with its own size indicator and an optional trailer containing
 * entity-header fields.
 *
 * The format is as follows:
 *
 * Chunked-Body =
 * chunk
 * last-chunk
 * trailer
 * CRLF
 *
 * chunk =
 * chunk-size [chunk-extension] CRLF
 * chunk-data CRLF
 *
 * chunk-size = 1*HEX
 * last-chunk = 1*("0") [chunk-extension] CRLF
 *
 * chunk-extension =
 * ( ";" chunk-ext-name [ "=" chunk-ext-val ] )
 *
 * chunk-ext-name = token
 * chunk-ext-val = token | quoted-string
 * chunk-data = chunk-size(OCTET)
 *
 * trailer = *(entity-header CRLF)
 *
 * The process for decoding "chunked" transfer-coding is as follows:
 *
 * length := 0
 *
 * read chunk-size, chunk-extension (if any) and CRLF
 * while(chunk-size > 0)
 * {
 *    read chunk-data and CRLF
 *    append chunk-data to entity-body
 *    length := length + chunk-size
 *    read chunk-size and CRLF
 * }
 *
 * read entity-header
 * while(entity-header not empty)
 * {
 *    append entity-header to existing header fields
 *    read entity-header
 * }
 *
 * Content-Length := length
 * Remove "chunked" from Transfer-Encoding
 *
 * Information from:
 * http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
 * http://www.w3.org/Protocols/rfc2616/rfc2616-sec19.html#sec19.4.5
 *
 * @author Dave Longley
 */
class HttpChunkedTransferInputStream : public db::io::PeekInputStream
{
protected:
   /**
    * The HttpTrailer to use for header trailers.
    */
   HttpTrailer* mTrailer;

   /**
    * Stores the number of bytes left to read for the current chunk.
    */
   int mChunkBytesLeft;

   /**
    * Set to true if the last chunk is being processed.
    */
   bool mLastChunk;

   /**
    * Stores the thread reading from this stream.
    */
   db::rt::Thread* mThread;

public:
   /**
    * Creates a new HttpChunkedTransferInputStream.
    *
    * @param is the ConnectionInputStream to receive data over.
    * @param trailer the HttpTrailer to store the header trailers in.
    */
   HttpChunkedTransferInputStream(
      db::net::ConnectionInputStream* is, HttpTrailer* trailer);

   /**
    * Destructs this HttpChunkedTransferInputStream.
    */
   virtual ~HttpChunkedTransferInputStream();

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
