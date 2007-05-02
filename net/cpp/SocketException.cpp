/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SocketException.h"

using namespace std;
using namespace db::net;
using namespace db::rt;

SocketException::SocketException(const string& message, const string& code) :
   IOException(message, code)
{
}

SocketException::~SocketException()
{
}
