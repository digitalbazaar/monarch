/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "InterruptedException.h"

using namespace std;
using namespace db::rt;

InterruptedException::InterruptedException(string message, string code) :
   Exception(message, code)
{
}
