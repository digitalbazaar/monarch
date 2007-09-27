/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/regex/InvalidRegexException.h"

using namespace db::rt;
using namespace db::util::regex;

InvalidRegexException::InvalidRegexException(
   const char* message, const char* type, int code) :
   Exception(message, type, code)
{
}

InvalidRegexException::~InvalidRegexException()
{
}
