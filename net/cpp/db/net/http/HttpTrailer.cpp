/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpTrailer.h"

using namespace db::net::http;

HttpTrailer::HttpTrailer()
{
   mContentLength = 0;
}

HttpTrailer::~HttpTrailer()
{
}

void HttpTrailer::update(unsigned long long contentLength)
{
   // save content length
   mContentLength += contentLength;
}

unsigned long long HttpTrailer::getContentLength()
{
   return mContentLength;
}
