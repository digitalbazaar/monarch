/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/http/HttpRequest.h"

#include "db/http/HttpConnection.h"
#include "db/http/HttpResponse.h"

using namespace db::io;
using namespace db::http;

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
   return mConnection;
}
