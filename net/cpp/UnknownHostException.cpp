/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "UnknownHostException.h"

using namespace db::net;

UnknownHostException::UnknownHostException(
   const char* message, const char* code) : Exception(message, code)
{
}

UnknownHostException::~UnknownHostException()
{
}
