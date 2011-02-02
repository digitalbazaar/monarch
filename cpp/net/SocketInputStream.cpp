/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/SocketInputStream.h"

using namespace monarch::io;
using namespace monarch::net;

SocketInputStream::SocketInputStream(Socket* s) :
   mSocket(s)
{
}

SocketInputStream::~SocketInputStream()
{
}

int SocketInputStream::read(char* b, int length)
{
   // read from the socket
   return mSocket->receive(b, length);
}
