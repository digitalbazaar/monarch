/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "ConnectionAcceptor.h"

using namespace db::net;

ConnectionAcceptor::ConnectionAcceptor(Socket* s, ConnectionHandler* h)
{
   mSocket = s;
   mHandler = h;
}

ConnectionAcceptor::~ConnectionAcceptor()
{
}

void ConnectionAcceptor::run()
{
   // accept a connection
   Socket* s = mSocket->accept(1);
   if(s != NULL)
   {
      // create Connection from connected Socket
      mHandler->createConnection(s);
   }
}
