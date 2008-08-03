/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpRequest.h"

#include "db/net/http/HttpConnection.h"
#include "db/net/http/HttpResponse.h"

using namespace db::io;
using namespace db::net;
using namespace db::net::http;

HttpRequest::HttpRequest(HttpConnection* hc)
{
   mConnection = hc;
}

HttpRequest::~HttpRequest()
{
}

inline HttpResponse* HttpRequest::createResponse()
{
   return new HttpResponse(this);
}

inline bool HttpRequest::sendHeader()
{
   return getConnection()->sendHeader(getHeader());
}

inline bool HttpRequest::receiveHeader()
{
   return getConnection()->receiveHeader(getHeader());
}

inline bool HttpRequest::sendBody(InputStream* is, HttpTrailer* trailer)
{
   return getConnection()->sendBody(getHeader(), is, trailer);
}

inline OutputStream* HttpRequest::getBodyOutputStream(HttpTrailer* trailer)
{
   return getConnection()->getBodyOutputStream(getHeader(), trailer);
}

inline bool HttpRequest::receiveBody(OutputStream* os, HttpTrailer* trailer)
{
   return getConnection()->receiveBody(getHeader(), os, trailer);
}

inline InputStream* HttpRequest::getBodyInputStream(HttpTrailer* trailer)
{
   return getConnection()->getBodyInputStream(getHeader(), trailer);
}

inline HttpRequestHeader* HttpRequest::getHeader()
{
   return &mHeader;
}

inline HttpConnection* HttpRequest::getConnection()
{
   return (HttpConnection*)mConnection;
}
