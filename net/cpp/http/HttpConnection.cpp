/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpConnection.h"
#include "HttpRequest.h"

using namespace std;
using namespace db::net;
using namespace db::net::http;

HttpConnection::HttpConnection(Connection* c, bool cleanup) :
   WebConnection(c, cleanup)
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
