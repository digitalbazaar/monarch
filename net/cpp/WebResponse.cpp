/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "WebResponse.h"
#include "WebRequest.h"

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
