/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SocketTimeoutException.h"

using namespace std;
using namespace db::net;

SocketTimeoutException::SocketTimeoutException(
   const string& message, const string& code) : SocketException(message, code)
{
}

SocketTimeoutException::~SocketTimeoutException()
{
}
