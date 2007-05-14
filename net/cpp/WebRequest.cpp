/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "WebRequest.h"
#include "WebConnection.h"

using namespace db::net;

WebRequest::WebRequest(WebConnection* wc)
{
   mConnection = wc;
}

WebRequest::~WebRequest()
{
}
