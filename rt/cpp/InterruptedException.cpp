/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "InterruptedException.h"

using namespace std;
using namespace db::rt;

InterruptedException::InterruptedException(
   const string& message, const string& code) : Exception(message, code)
{
}

InterruptedException::~InterruptedException()
{
}
