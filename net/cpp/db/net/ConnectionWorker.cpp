/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/ConnectionWorker.h"
#include "db/net/ConnectionService.h"

using namespace db::net;
using namespace db::modest;

ConnectionWorker::ConnectionWorker(ConnectionService* s, Connection* c)
{
   mService = s;
   mConnection = c;
}

ConnectionWorker::~ConnectionWorker()
{
   // ensure the connection is closed and delete it
   mConnection->close();
   delete mConnection;
   
   // delete operation
   delete mOperation;
}

void ConnectionWorker::run()
{
   // service the connection
   mService->serviceConnection(mConnection);
}

void ConnectionWorker::setOperation(Operation* op)
{
   mOperation = op;
}

Operation* ConnectionWorker::getOperation()
{
   return mOperation;
}
