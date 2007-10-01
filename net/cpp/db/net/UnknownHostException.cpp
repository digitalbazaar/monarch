/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/UnknownHostException.h"

using namespace db::net;

UnknownHostException::UnknownHostException(
   const char* message, const char* type, int code) :
   Exception(message, type, code)
{
}

UnknownHostException::~UnknownHostException()
{
}
