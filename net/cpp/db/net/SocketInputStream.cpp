/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/SocketInputStream.h"

using namespace db::io;
using namespace db::net;

SocketInputStream::SocketInputStream(Socket* s)
{
   mSocket = s;
}

SocketInputStream::~SocketInputStream()
{
}

int SocketInputStream::read(char* b, int length)
{
   // read from the socket
   return mSocket->receive(b, length);
}
