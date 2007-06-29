/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef HttpTransferChunkedOutputStream_H
#define HttpTransferChunkedOutputStream_H

#include "FilterOutputStream.h"
#include "ConnectionOutputStream.h"
#include "HttpHeader.h"

namespace db
{
namespace net
{
namespace http
{

/**
 * A HttpTransferChunkedOutputStream is a class that is used to encode
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
class HttpTransferChunkedOutputStream :
public virtual db::io::FilterOutputStream
{
protected:
   /**
    * The HttpHeader to use.
    */
   HttpHeader* mHeader;
   
public:
   /**
    * Creates a new HttpTransferChunkedOutputStream.
    * 
    * @param os the underlying ConnectionOutputStream to wrap.
    * @param header the HttpHeader to use.
    */
   HttpTransferChunkedOutputStream(
      db::net::ConnectionOutputStream* os, HttpHeader* header);
   
   /**
    * Destructs this HttpTransferChunkedOutputStream.
    */
   virtual ~HttpTransferChunkedOutputStream();
   
   /**
    * Writes some bytes to the stream.
    * 
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    * 
    * @return true if the write was successful, false if an IO exception
    *         occurred. 
    */
   virtual bool write(const char* b, unsigned int length);
   
   /**
    * Closes the stream.
    */
   virtual void close();
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
