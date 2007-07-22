/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "ConnectionHandler.h"
#include "ConnectionAcceptor.h"
#include "Server.h"
#include "TcpSocket.h"
#include "Convert.h"

using namespace std;
using namespace db::modest;
using namespace db::net;
using namespace db::rt;
using namespace db::util;

ConnectionHandler::ConnectionHandler(
   Server* server, ConnectionServicer* cs, InternetAddress* address) :
   mServicerOperations(true), mDataPresenterList(false)
{
   mServer = server;
   mServicer = cs;
   mAddress = address;
   mMaxConnectionCount = 10000;
   mConnectionCount = 0;
}

ConnectionHandler::~ConnectionHandler()
{
}

void ConnectionHandler::cleanupConnectionWorkers()
{
   for(list<ConnectionWorker*>::iterator i = mWorkers.begin();
       i != mWorkers.end();)
   {
      ConnectionWorker* cw = *i;
      if(cw->getOperation()->finished() || cw->getOperation()->canceled())
      {
         // delete the worker
         delete cw;
         i = mWorkers.erase(i);
      }
      else
      {
         i++;
      }
   }
}

bool ConnectionHandler::canExecuteOperation(ImmutableState* s)
{
   bool rval = false;
   
   // base key
   string key = "com.db.net.server.connections";
   
   // get permit counts for server and handler
   int serverPermits = mServer->getMaxConnectionCount();
   int handlerPermits = mMaxConnectionCount;
   string port = Convert::integerToString(getAddress()->getPort());
   s->getInteger(key + ".permits", serverPermits);
   s->getInteger(key + ".ports." + port + ".permits", handlerPermits);
   
   // subtract current connection counts
   serverPermits = (mServer->getConnectionCount() > serverPermits) ?
      0 : serverPermits -= mServer->getConnectionCount();
   handlerPermits = (mConnectionCount > handlerPermits) ?
      0 : handlerPermits -= mConnectionCount;
   
   // can only execute if permits exist for both server and handler
   if(serverPermits > 0 && handlerPermits > 0)
   {
      rval = true;
   }
   
   return rval;
}

bool ConnectionHandler::mustCancelOperation(ImmutableState* s)
{
   // cancel accepting connections if server is no longer running
   return !mServer->isRunning();
}

void ConnectionHandler::mutatePreExecutionState(State* s, Operation* op)
{
   // base key
   string key = "com.db.net.server.connections";
   
   // get permit counts for server and handler
   int serverPermits = mServer->getMaxConnectionCount();
   int handlerPermits = mMaxConnectionCount;
   string port = Convert::integerToString(getAddress()->getPort());
   s->getInteger(key + ".permits", serverPermits);
   s->getInteger(key + ".ports." + port + ".permits", handlerPermits);
   
   // decrement permits
   s->setInteger(key + ".permits", serverPermits - 1);
   s->setInteger(key + ".ports." + port + ".permits", handlerPermits - 1);
}

void ConnectionHandler::mutatePostExecutionState(State* s, Operation* op)
{
   // base key
   string key = "com.db.net.server.connections";
   
   // get permit counts for server and handler
   int serverPermits = mServer->getMaxConnectionCount();
   int handlerPermits = mMaxConnectionCount;
   string port = Convert::integerToString(getAddress()->getPort());
   s->getInteger(key + ".permits", serverPermits);
   s->getInteger(key + ".ports." + port + ".permits", handlerPermits);
   
   // increment permits
   s->setInteger(key + ".permits", serverPermits + 1);
   s->setInteger(key + ".ports." + port + ".permits", handlerPermits + 1);
}

void ConnectionHandler::run()
{
   // no connections yet
   mConnectionCount = 0;
   
   // create tcp socket
   TcpSocket s;
   
   // bind socket to the address and start listening
   if(s.bind(mAddress) && s.listen())
   {
      // create a connection acceptor
      ConnectionAcceptor ca(&s, this);
      
      Thread* t = Thread::currentThread();
      while(!t->isInterrupted())
      {
         // run accept operation
         Operation op(&ca, this, this);
         mServer->getKernel()->getEngine()->queue(&op);
         
         // prune servicer operations, clean up workers
         mServicerOperations.prune();
         cleanupConnectionWorkers();
         
         // wait for operation to complete
         op.waitFor();
      }
   }
   
   // close socket
   s.close();
   
   // terminate servicer operations, clean up workers
   mServicerOperations.terminate();
   cleanupConnectionWorkers();
}

void ConnectionHandler::createConnection(Socket* s)
{
   // increase connection count
   mServer->mConnectionCount++;
   mConnectionCount++;
   
   // wrap the Socket and create a connection
   Connection* c = new Connection(
      mDataPresenterList.createPresentationWrapper(s), true);
   
   // create ConnectionWorker and Operation to run it
   ConnectionWorker* worker = new ConnectionWorker(this, c);
   Operation* op = new Operation(worker, NULL, NULL);
   worker->setOperation(op);
   mServicerOperations.add(op);
   mWorkers.push_back(worker);
   
   // queue operation for execution
   mServer->getKernel()->getEngine()->queue(op);
}

void ConnectionHandler::serviceConnection(Connection* c)
{
   // service the connection
   mServicer->serviceConnection(c);
   
   // ensure connection is closed
   c->close();
   
   // decrease connection count
   mServer->mConnectionCount--;
   mConnectionCount--;
}

ConnectionServicer* ConnectionHandler::getServicer()
{
   return mServicer;
}

void ConnectionHandler::setMaxConnectionCount(unsigned int count)
{
   mMaxConnectionCount = count;
}

unsigned int ConnectionHandler::getMaxConnectionCount()
{
   return mMaxConnectionCount;
}

unsigned int ConnectionHandler::getConnectionCount()
{
   return mConnectionCount;
}

InternetAddress* ConnectionHandler::getAddress()
{
   return mAddress;
}
