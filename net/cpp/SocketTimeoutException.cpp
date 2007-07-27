/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SocketTimeoutException.h"

using namespace db::net;

SocketTimeoutException::SocketTimeoutException(
   const char* message, const char* code) : SocketException(message, code)
{
}

SocketTimeoutException::~SocketTimeoutException()
{
}
