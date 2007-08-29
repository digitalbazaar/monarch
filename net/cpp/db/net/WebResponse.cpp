/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/WebResponse.h"
#include "db/net/WebRequest.h"

using namespace db::net;

WebResponse::WebResponse(WebRequest* request)
{
   mRequest = request;
}

WebResponse::~WebResponse()
{
}

WebConnection* WebResponse::getWebConnection()
{
   return mRequest->getWebConnection();
}
