/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "ConnectionService.h"
#include "ConnectionAcceptor.h"
#include "Server.h"
#include "TcpSocket.h"
#include "Convert.h"

using namespace std;
using namespace db::modest;
using namespace db::net;
using namespace db::rt;
using namespace db::util;

ConnectionService::ConnectionService(
   Server* server,
   InternetAddress* address,
   ConnectionServicer* servicer,
   SocketDataPresenter* presenter) :
   PortService(server, address), mRunningServicers(false)
{
   mServicer = servicer;
   mDataPresenter = presenter;
   mSocket = NULL;
   mMaxConnectionCount = 10000;
   mConnectionCount = 0;
}

ConnectionService::~ConnectionService()
{
   // ensure service is stopped
   ConnectionService::stop();
}

Operation* ConnectionService::initialize()
{
   Operation* rval = NULL;
   
   // no connections yet
   mConnectionCount = 0;
   
   // create tcp socket
   mSocket = new TcpSocket();
   
   // bind socket to the address and start listening
   if(mSocket->bind(mAddress) && mSocket->listen())
   {
      // create Operation for running service
      rval = new Operation(this, NULL, NULL);
   }
   
   return rval;
}

void ConnectionService::cleanup()
{
   if(mSocket != NULL)
   {
      // clean up socket
      delete mSocket;
      mSocket = NULL;
   }
}

void ConnectionService::cleanupWorkers()
{
   for(list<ConnectionWorker*>::iterator i = mWorkers.begin();
       i != mWorkers.end();)
   {
      ConnectionWorker* cw = *i;
      if(cw->getOperation()->stopped())
      {
         // remove the operation from the running servicers list
         mRunningServicers.remove(cw->getOperation());
         
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

bool ConnectionService::canExecuteOperation(ImmutableState* s)
{
   bool rval = false;
   
   // base key
   string key = "com.db.net.server.connections";
   
   // get permit counts for server and service
   int serverPermits = mServer->getMaxConnectionCount();
   int servicePermits = mMaxConnectionCount;
   string port = Convert::integerToString(mAddress->getPort());
   s->getInteger(key + ".permits", serverPermits);
   s->getInteger(key + ".ports." + port + ".permits", servicePermits);
   
   // subtract current connection counts
   serverPermits = (mServer->getConnectionCount() > serverPermits) ?
      0 : serverPermits -= mServer->getConnectionCount();
   servicePermits = (mConnectionCount > servicePermits) ?
      0 : servicePermits -= mConnectionCount;
   
   // can only execute if permits exist for both server and service
   if(serverPermits > 0 && servicePermits > 0)
   {
      rval = true;
   }
   
   return rval;
}

bool ConnectionService::mustCancelOperation(ImmutableState* s)
{
   // cancel accepting connections if server is no longer running
   return !mServer->isRunning();
}

void ConnectionService::mutatePreExecutionState(State* s, Operation* op)
{
   // base key
   string key = "com.db.net.server.connections";
   
   // get permit counts for server and service
   int serverPermits = mServer->getMaxConnectionCount();
   int servicePermits = mMaxConnectionCount;
   string port = Convert::integerToString(mAddress->getPort());
   s->getInteger(key + ".permits", serverPermits);
   s->getInteger(key + ".ports." + port + ".permits", servicePermits);
   
   // decrement permits
   s->setInteger(key + ".permits", serverPermits - 1);
   s->setInteger(key + ".ports." + port + ".permits", servicePermits - 1);
}

void ConnectionService::mutatePostExecutionState(State* s, Operation* op)
{
   // base key
   string key = "com.db.net.server.connections";
   
   // get permit counts for server and service
   int serverPermits = mServer->getMaxConnectionCount();
   int servicePermits = mMaxConnectionCount;
   string port = Convert::integerToString(mAddress->getPort());
   s->getInteger(key + ".permits", serverPermits);
   s->getInteger(key + ".ports." + port + ".permits", servicePermits);
   
   // increment permits
   s->setInteger(key + ".permits", serverPermits + 1);
   s->setInteger(key + ".ports." + port + ".permits", servicePermits + 1);
}

void ConnectionService::run()
{
   // create a connection acceptor
   ConnectionAcceptor ca(mSocket, this);
   
   while(!Operation::interrupted())
   {
      // run accept operation
      Operation op(&ca, this, this);
      mServer->getKernel()->getEngine()->queue(&op);
      
      // prune running servicers, clean up workers
      mRunningServicers.prune();
      cleanupWorkers();
      
      // wait for operation to complete, do not allow interruptions
      op.waitFor(false);
   }
   
   // close socket
   mSocket->close();
   
   // terminate running servicers, clean up workers
   mRunningServicers.terminate();
   cleanupWorkers();
}

void ConnectionService::createConnection(Socket* s)
{
   // try to wrap Socket for standard data presentation
   bool secure = false;
   Socket* wrapper = s;
   if(mDataPresenter != NULL)
   {
      wrapper = mDataPresenter->createPresentationWrapper(s, secure);
   }
   
   if(wrapper != NULL)
   {
      // create connection
      Connection* c = new Connection(wrapper, true);
      c->setSecure(secure);
      
      // increase connection count
      mServer->mConnectionCount++;
      mConnectionCount++;
      
      // create ConnectionWorker and Operation to run it
      ConnectionWorker* worker = new ConnectionWorker(this, c);
      Operation* op = new Operation(worker, NULL, NULL);
      worker->setOperation(op);
      mRunningServicers.add(op);
      mWorkers.push_back(worker);
      
      // queue operation for execution
      mServer->getKernel()->getEngine()->queue(op);
   }
   else
   {
      // close socket, data cannot be presented in standard format
      s->close();
      delete s;
   }
}

void ConnectionService::serviceConnection(Connection* c)
{
   // service the connection
   mServicer->serviceConnection(c);
   
   // ensure connection is closed
   c->close();
   
   // decrease connection count
   mServer->mConnectionCount--;
   mConnectionCount--;
}

void ConnectionService::setMaxConnectionCount(unsigned int count)
{
   mMaxConnectionCount = count;
}

unsigned int ConnectionService::getMaxConnectionCount()
{
   return mMaxConnectionCount;
}

unsigned int ConnectionService::getConnectionCount()
{
   return mConnectionCount;
}

string& ConnectionService::toString(string& str)
{
   string port = Convert::integerToString(mAddress->getPort());
   str = "ConnectionService [" + mAddress->getHost() + ":" + port + "]";
   return str;
}
