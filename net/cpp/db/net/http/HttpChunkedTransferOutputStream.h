/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpChunkedTransferOutputStream_H
#define db_net_http_HttpChunkedTransferOutputStream_H

#include "db/io/BufferedOutputStream.h"
#include "db/net/ConnectionOutputStream.h"
#include "db/net/http/HttpTrailer.h"

namespace db
{
namespace net
{
namespace http
{

/**
 * A HttpChunkedTransferOutputStream is a class that is used to encode
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
class HttpChunkedTransferOutputStream :
public db::io::BufferedOutputStream
{
protected:
   /**
    * The output buffer.
    */
   db::io::ByteBuffer mOutputBuffer;
   
   /**
    * The HttpTrailer to use for header trailers.
    */
   HttpTrailer* mTrailer;
   
   /**
    * Stores the amount of data sent to include in the header trailers.
    */
   unsigned long long mDataSent;
   
   /**
    * True if finished writing output, false if not.
    */
   bool mFinished;
   
public:
   /**
    * Creates a new HttpChunkedTransferOutputStream.
    * 
    * @param os the ConnectionOutputStream to send data over.
    * @param trailer the HttpTrailer to use for header trailers.
    */
   HttpChunkedTransferOutputStream(
      ConnectionOutputStream* os, HttpTrailer* trailer);
   
   /**
    * Destructs this HttpChunkedTransferOutputStream.
    */
   virtual ~HttpChunkedTransferOutputStream();
   
   /**
    * Forces this stream to flush its output, if any of it was buffered.
    * 
    * @return true if the write was successful, false if an IO exception
    *         occurred. 
    */
   virtual bool flush();
   
   /**
    * Forces this stream to finish its output, if the stream has such a
    * function.
    * 
    * @return true if the write was successful, false if an IO exception
    *         occurred.
    */
   virtual bool finish();
   
   /**
    * Closes the stream.
    */
   virtual void close();
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
