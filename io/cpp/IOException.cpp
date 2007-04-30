/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "IOException.h"

using namespace std;
using namespace db::io;
using namespace db::rt;

IOException::IOException(string message, string code) :
   Exception(message, code)
{
}
