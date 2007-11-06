/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpResponse.h"
#include "db/net/http/HttpRequest.h"

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

IOException* HttpResponse::sendBody(InputStream* is, HttpTrailer* trailer)
{
   return getConnection()->sendBody(getHeader(), is, trailer);
}

OutputStream* HttpResponse::getBodyOutputStream(HttpTrailer* trailer)
{
   return getConnection()->getBodyOutputStream(getHeader(), trailer);
}

IOException* HttpResponse::receiveBody(OutputStream* os, HttpTrailer* trailer)
{
   return getConnection()->receiveBody(getHeader(), os, trailer);
}

HttpResponseHeader* HttpResponse::getHeader()
{
   return &mHeader;
}

HttpConnection* HttpResponse::getConnection()
{
   return (HttpConnection*)getWebConnection();
}
