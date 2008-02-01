/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpTrailer_H
#define db_net_http_HttpTrailer_H

#include "db/net/http/HttpHeader.h"
#include "db/rt/Collectable.h"

namespace db
{
namespace net
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
    * method will be called before a trailer is sent out.
    * 
    * @param contentLength the length of the content that was sent.
    */
   virtual void update(unsigned long long contentLength);
};

// typedef for a counted reference to an HttpTrailer
typedef db::rt::Collectable<HttpTrailer> HttpTrailerRef;

} // end namespace http
} // end namespace net
} // end namespace db
#endif
