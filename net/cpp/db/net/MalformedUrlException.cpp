/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/MalformedUrlException.h"

using namespace db::net;

MalformedUrlException::MalformedUrlException(
   const char* message, const char* type, int code) :
   Exception(message, type, code)
{
}

MalformedUrlException::~MalformedUrlException()
{
}
