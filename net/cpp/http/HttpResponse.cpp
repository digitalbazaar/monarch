/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpResponse.h"
#include "HttpRequest.h"

using namespace db::net;
using namespace db::net::http;

HttpResponse::HttpResponse(HttpRequest* request) : WebResponse(request)
{
}

HttpResponse::~HttpResponse()
{
}

HttpResponseHeader* HttpResponse::getHeader()
{
   return &mHeader;
}
