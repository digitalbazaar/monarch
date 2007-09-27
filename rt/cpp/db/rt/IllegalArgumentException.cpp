/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/IllegalArgumentException.h"

using namespace db::rt;

IllegalArgumentException::IllegalArgumentException(
   const char* message, const char* type, int code) :
   Exception(message, type, code)
{
}

IllegalArgumentException::~IllegalArgumentException()
{
}
