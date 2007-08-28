/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpResponse.h"
#include "HttpRequest.h"

using namespace db::io;
using namespace db::net;
using namespace db::net::http;

HttpResponse::HttpResponse(HttpRequest* request) : WebResponse(request)
{
}

HttpResponse::~HttpResponse()
{
}

IOException* HttpResponse::sendHeader()
{
   return getConnection()->sendHeader(getHeader());
}

IOException* HttpResponse::receiveHeader()
{
   return getConnection()->receiveHeader(getHeader());
}

IOException* HttpResponse::sendBody(InputStream* is, HttpHeader* trailers)
{
   return getConnection()->sendBody(getHeader(), is, trailers);
}

IOException* HttpResponse::receiveBody(OutputStream* os, HttpHeader* trailers)
{
   return getConnection()->receiveBody(getHeader(), os, trailers);
}

HttpResponseHeader* HttpResponse::getHeader()
{
   return &mHeader;
}

HttpConnection* HttpResponse::getConnection()
{
   return (HttpConnection*)getWebConnection();
}
