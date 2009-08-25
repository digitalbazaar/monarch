/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/http/HttpTrailer.h"

using namespace db::http;

HttpTrailer::HttpTrailer()
{
   mContentLength = 0;
}

HttpTrailer::~HttpTrailer()
{
}

inline void HttpTrailer::update(unsigned long long contentLength)
{
   // save content length
   setContentLength(contentLength);
}

inline void HttpTrailer::setContentLength(unsigned long long contentLength)
{
   mContentLength = contentLength;
}

inline unsigned long long HttpTrailer::getContentLength()
{
   return mContentLength;
}

inline HttpHeader::Type HttpTrailer::getType()
{
   return HttpHeader::Trailer;
}
