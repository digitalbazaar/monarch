/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SocketOutputStream.h"

using namespace db::io;
using namespace db::net;

SocketOutputStream::SocketOutputStream(Socket* s)
{
   mSocket = s;
}

SocketOutputStream::~SocketOutputStream()
{
}

inline void SocketOutputStream::write(const char& b) throw(IOException)
{
   char buffer[1] = {b};
   write(buffer, 0, 1);
}

inline void SocketOutputStream::write(
   char* b, unsigned int offset, unsigned int length) throw(IOException)
{
   // send data through the socket
   mSocket->send(b, offset, length);
}
