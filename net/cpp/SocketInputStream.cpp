/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SocketInputStream.h"

using namespace db::io;
using namespace db::net;

SocketInputStream::SocketInputStream(Socket* s)
{
   mSocket = s;
}

SocketInputStream::~SocketInputStream()
{
}

bool SocketInputStream::read(char& b) throw(IOException)
{
   bool rval = false;
   
   char buffer[1];
   if(read(buffer, 0, 1) != -1)
   {
      b = buffer[0];
   }
   
   return rval;
}

int SocketInputStream::read(
   char* b, unsigned int offset, unsigned int length) throw(IOException)
{
   // read from the socket
   return mSocket->receive(b, offset, length);
}
