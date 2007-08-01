/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "WebRequest.h"

using namespace db::net;

WebRequest::WebRequest(WebConnection* wc)
{
   mConnection = wc;
}

WebRequest::~WebRequest()
{
}

WebConnection* WebRequest::getWebConnection()
{
   return mConnection;
}
