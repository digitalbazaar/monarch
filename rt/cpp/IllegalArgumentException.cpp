/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "IllegalArgumentException.h"

using namespace std;
using namespace db::rt;

IllegalArgumentException::IllegalArgumentException(
   const string& message, const string& code) : Exception(message, code)
{
}

IllegalArgumentException::~IllegalArgumentException()
{
}
