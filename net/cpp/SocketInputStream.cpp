/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SocketInputStream.h"
#include "Socket.h"

using namespace db::net;

SocketInputStream::SocketInputStream(Socket* s)
{
   mSocket = s;
}

SocketInputStream::~SocketInputStream()
{
}

inline bool SocketInputStream::read(char& b)
{
   bool rval = false;
   
   char buffer[1];
   if(read(buffer, 0, 1) != -1)
   {
      b = buffer[0];
   }
   
   return rval;
}

inline int SocketInputStream::read(
   char* b, unsigned int offset, unsigned int length)
{
   return mSocket->read(b, offset, length);
}
