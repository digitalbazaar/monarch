/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/SocketOutputStream.h"

using namespace db::io;
using namespace db::net;

SocketOutputStream::SocketOutputStream(Socket* s)
{
   mSocket = s;
}

SocketOutputStream::~SocketOutputStream()
{
}

inline bool SocketOutputStream::write(const char* b, int length)
{
   // send data through the socket
   return mSocket->send(b, length);
}
