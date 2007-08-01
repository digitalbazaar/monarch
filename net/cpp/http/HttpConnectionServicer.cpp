/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpConnectionServicer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

using namespace std;
using namespace db::net::http;

HttpConnectionServicer::HttpConnectionServicer()
{
}

HttpConnectionServicer::~HttpConnectionServicer()
{
}

void HttpConnectionServicer::serviceConnection(Connection* c)
{
   // wrap connection
   HttpConnection hc(c, false);
   
   // create request
   HttpRequest* request = (HttpRequest*)hc.createRequest();
   
   // FIXME: service request
   
   // clean up request
   delete request;
   
   
   // FIXME: implement me
}
