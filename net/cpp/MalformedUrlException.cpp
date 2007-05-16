/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "MalformedUrlException.h"

using namespace std;
using namespace db::net;

MalformedUrlException::MalformedUrlException(
   const string& message, const string& code) : Exception(message, code)
{
}

MalformedUrlException::~MalformedUrlException()
{
}
