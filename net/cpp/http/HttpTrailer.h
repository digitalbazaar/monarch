/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpTrailer_H
#define db_net_http_HttpTrailer_H

#include "HttpHeader.h"

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
    * method will be called before a trailer is sent out. The default
    * implementation sets the Content-Length field in the trailer.
    * 
    * @param contentLength the length of the content that was sent.
    */
   virtual void update(unsigned long long contentLength);
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
