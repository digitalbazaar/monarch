/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "ConnectionService.h"
#include "Server.h"
#include "TcpSocket.h"

using namespace std;
using namespace db::modest;
using namespace db::net;
using namespace db::rt;

ConnectionService::ConnectionService(
   Server* server,
   InternetAddress* address,
   ConnectionServicer* servicer,
   SocketDataPresenter* presenter) :
   PortService(server, address),
   mConnectionSemaphore(10000, true), mRunningServicers(false)
{
   mServicer = servicer;
   mDataPresenter = presenter;
   mSocket = NULL;
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
   if(mSocket->bind(getAddress()) && mSocket->listen())
   {
      // create Operation for running service
      rval = new Operation(this, this, NULL);
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
   // can execute if server is running
   return mServer->isRunning();
}

bool ConnectionService::mustCancelOperation(ImmutableState* s)
{
   // must cancel if server is no longer running
   return !mServer->isRunning();
}

void ConnectionService::run()
{
   while(!mOperation->isInterrupted())
   {
      // acquire service connection permit
      if(mConnectionSemaphore.acquire() == NULL)
      {
         // acquire server connection permit
         if(mServer->mConnectionSemaphore.acquire() == NULL)
         {
            Socket* s = mSocket->accept(0);
            if(s != NULL)
            {
               // create Connection from the connected Socket
               createConnection(s);
            }
            else
            {
               // release connection permits
               mServer->mConnectionSemaphore.release();
               mConnectionSemaphore.release();
            }
         }
         else
         {
            // release service connection permit
            mConnectionSemaphore.release();
         }
      }
      
      // prune running servicers, clean up workers
      mRunningServicers.prune();
      cleanupWorkers();
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
      
      // release connection permits
      mServer->mConnectionSemaphore.release();
      mConnectionSemaphore.release();
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
   
   // release connection permits
   mServer->mConnectionSemaphore.release();
   mConnectionSemaphore.release();
}

void ConnectionService::setMaxConnectionCount(unsigned int count)
{
   mConnectionSemaphore.setMaxPermitCount(count);
}

unsigned int ConnectionService::getMaxConnectionCount()
{
   return mConnectionSemaphore.getMaxPermitCount();
}

unsigned int ConnectionService::getConnectionCount()
{
   return mConnectionCount;
}
