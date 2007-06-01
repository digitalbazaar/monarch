/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "InvalidRegexException.h"

using namespace std;
using namespace db::rt;
using namespace db::util::regex;

InvalidRegexException::InvalidRegexException(
   const string& message, const string& code) : Exception(message, code)
{
}

InvalidRegexException::~InvalidRegexException()
{
}
