/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_http_HttpTrailer_H
#define monarch_http_HttpTrailer_H

#include "monarch/http/HttpHeader.h"
#include "monarch/rt/Collectable.h"

namespace monarch
{
namespace http
{

/**
 * An HttpTrailer is an HTTP header that follows a body of content.
 *
 * Both http requests and responses use an HttpHeader. A request uses a
 * request-line for its start-line and a response uses a status-line for its
 * start-line.
 *
 * A multipart http message will use a MIME boundary as the start-line for
 * its HttpHeader.
 *
 * According to the HTTP/1.1 RFC trailers MUST NOT include the following
 * http header fields:
 *
 * Transfer-Encoding
 * Content-Length
 * Trailer
 *
 * @author Dave Longley
 */
class HttpTrailer : public HttpHeader
{
protected:
   /**
    * The content length for the associated message body.
    */
   int64_t mContentLength;

public:
   /**
    * Creates a new HttpTrailer.
    */
   HttpTrailer();

   /**
    * Destructs this HttpTrailer.
    */
   virtual ~HttpTrailer();

   /**
    * Updates this trailer according to some proprietary implementation. This
    * method will be called before a trailer is sent out or after it has been
    * received.
    *
    * @param contentLength the length of the content that was sent or received.
    */
   virtual void update(int64_t contentLength);

   /**
    * Explicitly set the content length in this trailer.
    *
    * @param contentLength the content length to set.
    */
   virtual void setContentLength(int64_t contentLength);

   /**
    * Gets the content length provided in an update to this trailer.
    *
    * @return the content length provided in an update to this trailer.
    */
   virtual int64_t getContentLength();

   /**
    * Gets the type of header this is.
    *
    * @return the type of header this is.
    */
   virtual Type getType();
};

// typedef for a counted reference to an HttpTrailer
typedef monarch::rt::Collectable<HttpTrailer> HttpTrailerRef;

} // end namespace http
} // end namespace monarch
#endif
