/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "UnknownHostException.h"

using namespace std;
using namespace db::net;

UnknownHostException::UnknownHostException(
   const string& message, const string& code) : Exception(message, code)
{
}

UnknownHostException::~UnknownHostException()
{
}
