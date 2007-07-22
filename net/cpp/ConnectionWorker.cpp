/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "ConnectionWorker.h"
#include "ConnectionHandler.h"

using namespace db::net;
using namespace db::modest;

ConnectionWorker::ConnectionWorker(ConnectionHandler* h, Connection* c)
{
   mHandler = h;
   mConnection = c;
}

ConnectionWorker::~ConnectionWorker()
{
   // ensure the connection is closed and delete it
   mConnection->close();
   delete mConnection;
}

void ConnectionWorker::run()
{
   // service the connection
   mHandler->serviceConnection(mConnection);
}

Connection* ConnectionWorker::getConnection()
{
   return mConnection;
}

void ConnectionWorker::setOperation(Operation* op)
{
   mOperation = op;
}

Operation* ConnectionWorker::getOperation()
{
   return mOperation;
}
