/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpConnection.h"
#include "HttpRequest.h"

using namespace db::net;
using namespace db::net::http;

HttpConnection::HttpConnection(Socket* s, bool cleanup) :
   WebConnection(s, cleanup)
{
}

HttpConnection::~HttpConnection()
{
}

WebRequest* HttpConnection::createRequest()
{
   // create HttpRequest
   return new HttpRequest(this);
}
