/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "InterruptedException.h"

using namespace db::rt;

InterruptedException::InterruptedException(
   const char* message, const char* code) : Exception(message, code)
{
}

InterruptedException::~InterruptedException()
{
}
