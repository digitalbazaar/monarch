/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Exception.h"

using namespace std;
using namespace db::rt;

Exception::Exception(string message, string code)
{
   // store message and code
   mMessage = message;
   mCode = code;
}

const string& Exception::getMessage()
{
   return mMessage;
}

const string& Exception::getCode()
{
   return mCode;
}
