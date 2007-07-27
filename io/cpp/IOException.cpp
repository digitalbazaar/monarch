/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "IOException.h"

using namespace db::io;
using namespace db::rt;

IOException::IOException(const char* message, const char* code) :
   Exception(message, code)
{
}

IOException::~IOException()
{
}
