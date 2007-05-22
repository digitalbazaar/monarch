/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpRequest.h"
#include "HttpConnection.h"
#include "HttpResponse.h"

using namespace db::net;
using namespace db::net::http;

HttpRequest::HttpRequest(HttpConnection* hc) : WebRequest(hc)
{
}

HttpRequest::~HttpRequest()
{
}

WebResponse* HttpRequest::createResponse()
{
   return new HttpResponse(this);
}

HttpRequestHeader HttpRequest::getHeader()
{
   return mHeader;
}
