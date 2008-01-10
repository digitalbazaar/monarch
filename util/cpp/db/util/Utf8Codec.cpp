/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/Utf8Codec.h"

using namespace std;
using namespace db::rt;
using namespace db::util;

Exception* Utf8Codec::encode(
   const char* ce, const char* data, unsigned int length, string& out)
{
   Exception* rval = NULL;
   
   // FIXME:
   
   return rval;
}

Exception* Utf8Codec::decode(
   const char* str, const char* ce, char** data, unsigned int& length)
{
   Exception* rval = NULL;
   
   // FIXME:
   *data = NULL;
   length = 0;
   
   return rval;
}
