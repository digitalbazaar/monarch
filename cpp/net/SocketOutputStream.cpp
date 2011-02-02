/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/SocketOutputStream.h"

using namespace monarch::io;
using namespace monarch::net;

SocketOutputStream::SocketOutputStream(Socket* s) :
   mSocket(s)
{
}

SocketOutputStream::~SocketOutputStream()
{
}

inline bool SocketOutputStream::write(const char* b, int length)
{
   // send data through the socket
   return mSocket->send(b, length);
}
