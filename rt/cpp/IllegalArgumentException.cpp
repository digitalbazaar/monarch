/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "IllegalArgumentException.h"

using namespace db::rt;

IllegalArgumentException::IllegalArgumentException(
   const char* message, const char* code) : Exception(message, code)
{
}

IllegalArgumentException::~IllegalArgumentException()
{
}
