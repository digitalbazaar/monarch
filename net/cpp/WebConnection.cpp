/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "WebConnection.h"

using namespace db::net;

WebConnection::WebConnection(Socket* s, bool cleanup) : Connection(s, cleanup)
{
}

WebConnection::~WebConnection()
{
}
