/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "WebResponse.h"
#include "WebConnection.h"

using namespace db::net;

WebResponse::WebResponse(WebRequest* request)
{
   mRequest = request;
}

WebResponse::~WebResponse()
{
}
