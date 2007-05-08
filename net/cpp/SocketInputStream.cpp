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
   
   if(read(&b, 1) != -1)
   {
      rval = true;
   }
   
   return rval;
}

int SocketInputStream::read(char* b, unsigned int length) throw(IOException)
{
   // read from the socket
   return mSocket->receive(b, length);
}
