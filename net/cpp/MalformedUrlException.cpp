/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "MalformedUrlException.h"

using namespace db::net;

MalformedUrlException::MalformedUrlException(
   const char* message, const char* code) : Exception(message, code)
{
}

MalformedUrlException::~MalformedUrlException()
{
}
