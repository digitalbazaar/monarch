/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/SocketOutputStream.h"

using namespace db::io;
using namespace db::net;

SocketOutputStream::SocketOutputStream(Socket* s)
{
   mSocket = s;
}

SocketOutputStream::~SocketOutputStream()
{
}

inline bool SocketOutputStream::write(const char* b, unsigned int length)
{
   // send data through the socket
   return mSocket->send(b, length);
}
