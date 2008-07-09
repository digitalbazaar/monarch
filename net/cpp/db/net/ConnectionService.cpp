/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/ConnectionService.h"

#include "db/net/Server.h"
#include "db/net/TcpSocket.h"
#include "db/rt/RunnableDelegate.h"

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
   mConnectionSemaphore(100, true)
{
   mServicer = servicer;
   mDataPresenter = presenter;
   mSocket = NULL;
}

ConnectionService::~ConnectionService()
{
   // ensure service is stopped
   ConnectionService::stop();
}

Operation ConnectionService::initialize()
{
   Operation rval(NULL);
   
   // create tcp socket
   mSocket = new TcpSocket();
   
   // bind socket to the address and start listening
   if(mSocket->bind(getAddress()) && mSocket->listen())
   {
      // create Operation for running service
      rval = *this;
      rval->addGuard(this);
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
   
   // release any used connection permits
   int used = mConnectionSemaphore.usedPermits();
   if(used > 0)
   {
      mConnectionSemaphore.release(used);
   }
}

bool ConnectionService::canExecuteOperation(ImmutableState* s, Operation& op)
{
   // can execute if server is running
   return mServer->isRunning();
}

bool ConnectionService::mustCancelOperation(ImmutableState* s, Operation& op)
{
   // must cancel if server is no longer running
   return !mServer->isRunning();
}

void ConnectionService::run()
{
   while(!mOperation->isInterrupted())
   {
      // prune running servicers
      mRunningServicers.prune();
      
      // acquire service connection permit
      if(mConnectionSemaphore.acquire())
      {
         // acquire server connection permit
         if(mServer->mConnectionSemaphore.acquire())
         {
            // wait for 5 seconds for a connection
            Socket* s = mSocket->accept(5);
            if(s == NULL || !createConnection(s))
            {
               // could not create connection, release connection permits
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
   }
   
   // close socket
   mSocket->close();
   
   // terminate running servicers
   mRunningServicers.terminate();
}

bool ConnectionService::createConnection(Socket* s)
{
   bool rval = true;
   
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
      
      // create RunnableDelegate to service connection and run as an Operation
      RunnableRef r =
         new RunnableDelegate<ConnectionService>(
            this, &ConnectionService::serviceConnection, c);
      Operation op(r);
      mRunningServicers.add(op);
      
      // run operation
      mServer->getOperationRunner()->runOperation(op);
   }
   else
   {
      // close socket, data cannot be presented in standard format
      s->close();
      delete s;
      rval = false;
   }
   
   return rval;
}

void ConnectionService::serviceConnection(void* c)
{
   // service the connection
   mServicer->serviceConnection((Connection*)c, this);
}

void ConnectionService::cleanupConnection(Connection* c)
{
   // ensure connection is closed
   c->close();
   
   // clean up connection
   delete c;
   
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
   return mConnectionSemaphore.usedPermits();
}
