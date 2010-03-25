/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpTrailer.h"

using namespace monarch::http;

HttpTrailer::HttpTrailer() :
   mContentLength(0)
{
}

HttpTrailer::~HttpTrailer()
{
}

inline void HttpTrailer::update(int64_t contentLength)
{
   // save content length
   setContentLength(contentLength);
}

inline void HttpTrailer::setContentLength(int64_t contentLength)
{
   mContentLength = contentLength;
}

inline int64_t HttpTrailer::getContentLength()
{
   return mContentLength;
}

inline HttpHeader::Type HttpTrailer::getType()
{
   return HttpHeader::Trailer;
}
