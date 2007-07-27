/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SocketException.h"

using namespace db::net;

SocketException::SocketException(const char* message, const char* code) :
   IOException(message, code)
{
}

SocketException::~SocketException()
{
}
