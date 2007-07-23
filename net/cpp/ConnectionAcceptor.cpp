/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "ConnectionAcceptor.h"

using namespace std;
using namespace db::net;

ConnectionAcceptor::ConnectionAcceptor(
   Socket* socket, ConnectionService* service)
{
   mSocket = socket;
   mService = service;
}

ConnectionAcceptor::~ConnectionAcceptor()
{
}
#include <iostream>
void ConnectionAcceptor::run()
{
   cout << ".................STARTING ACCEPTOR" << endl;
   
   // accept a connection
   Socket* s = mSocket->accept(1);
   if(s != NULL)
   {
      // create Connection from connected Socket
      mService->createConnection(s);
   }
   
   cout << ".................FINISHED ACCEPTOR" << endl;
}

string& ConnectionAcceptor::toString(string& str)
{
   str = "ConnectionAcceptor for " + mService->toString(str);
   return str;
}
