/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "InvalidRegexException.h"

using namespace db::rt;
using namespace db::util::regex;

InvalidRegexException::InvalidRegexException(
   const char* message, const char* code) : Exception(message, code)
{
}

InvalidRegexException::~InvalidRegexException()
{
}
