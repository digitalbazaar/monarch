/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/SocketException.h"

using namespace db::net;

SocketException::SocketException(
   const char* message, const char* type, int code) :
   IOException(message, type, code)
{
}

SocketException::~SocketException()
{
}
