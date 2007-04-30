/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SocketException.h"

using namespace std;
using namespace db::net;
using namespace db::rt;

SocketException::SocketException(string message, string code) :
   Exception(message, code)
{
}
