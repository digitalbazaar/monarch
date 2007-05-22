/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef HttpTransferChunkedInputStream_H
#define HttpTransferChunkedInputStream_H

#include "PeekInputStream.h"
#include "ConnectionInputStream.h"

namespace db
{
namespace net
{
namespace http
{

/**
 * A HttpTransferChunkedInputStream is a class that is used to decode
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
class HttpTransferChunkedInputStream : public virtual db::io::PeekInputStream
{
protected:
   /**
    * Stores the number of bytes left to read for the current chunk.
    */
   int mChunkBytesLeft;
   
   /**
    * Set to true if the last chunk is being processed.
    */
   bool mLastChunk;
   
public:
   /**
    * Creates a new HttpTransferChunkedInputStream.
    * 
    * @param is the underlying ConnectionInputStream to wrap.
    */
   HttpTransferChunkedInputStream(db::net::ConnectionInputStream* is);
   
   /**
    * Destructs this HttpTransferChunkedInputStream.
    */
   virtual ~HttpTransferChunkedInputStream();
   
   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of -1 will be returned if the end of the stream has been reached,
    * otherwise the number of bytes read will be returned.
    * 
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or -1 if the end of the
    *         stream has been reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual int read(char* b, unsigned int length) throw(db::io::IOException);
   
   /**
    * Closes the stream. This will not close the underlying http stream.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual void close() throw(db::io::IOException);
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
