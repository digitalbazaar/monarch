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
   PortService(server, address)
{
   mServicer = servicer;
   mDataPresenter = presenter;
   mSocket = NULL;
   
   mMaxConnections = 100;
   mCurrentConnections = 0;
}

ConnectionService::~ConnectionService()
{
   // ensure service is stopped
   ConnectionService::stop();
}

Operation ConnectionService::initialize()
{
   Operation rval(NULL);
   
   // no connections yet
   mCurrentConnections = 0;
   
   // create tcp socket
   mSocket = new TcpSocket();
   
   // bind socket to the address and start listening
   if(mSocket->bind(getAddress()) && mSocket->listen())
   {
      // create Operation for running service
      rval = *this;
      rval->setUserData((void*)"accept");
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
}

bool ConnectionService::canExecuteOperation(ImmutableState* s, Operation& op)
{
   bool rval = false;
   
   if(strcmp("accept", (const char*)op->getUserData()) == 0)
   {
      // accept OP can execute if server is running
      rval = mServer->isRunning();
   }
   else
   {
      // service OP can execute if the server and the connection service
      // have enough permits available
      int32_t permits =
         mServer->getMaxConnectionCount() - mServer->getConnectionCount();
      if(permits > 0)
      {
         permits = getMaxConnectionCount() - getConnectionCount();
         rval = (permits > 0);
      }
   }
   
   return rval;
}

bool ConnectionService::mustCancelOperation(ImmutableState* s, Operation& op)
{
   bool rval;
   
   if(strcmp("accept", (const char*)op->getUserData()) == 0)
   {
      // must cancel accept OP if server is no longer running
      rval = !mServer->isRunning();
   }
   else
   {
      // don't cancel service OPs, as they must cleanup
      rval = true;
   }
   
   return rval;
}

void ConnectionService::mutatePreExecutionState(State* s, Operation& op)
{
   // increase current connections
   mServer->mCurrentConnections++;
   mCurrentConnections++;
}

void ConnectionService::mutatePostExecutionState(State* s, Operation& op)
{
   // decrease current connections
   mCurrentConnections--;
   mServer->mCurrentConnections--;
}

void ConnectionService::run()
{
   Socket* s;
   while(!mOperation->isInterrupted())
   {
      // prune running servicers
      mRunningServicers.prune();
      
      // wait for 5 seconds for a connection
      if((s = mSocket->accept(5)) != NULL)
      {
         // create connection
         createConnection(s);
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
            this, &ConnectionService::serviceConnection, c,
            &ConnectionService::cleanupConnection);
      Operation op(r);
      op->setUserData((void*)"service");
      op->addGuard(this);
      op->addStateMutator(this);
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
   mServicer->serviceConnection((Connection*)c);
}

void ConnectionService::cleanupConnection(void* c)
{
   // ensure connection is closed
   // clean up connection
   Connection* conn = (Connection*)c;
   conn->close();
   delete conn;
}

inline void ConnectionService::setMaxConnectionCount(int32_t count)
{
   mMaxConnections = count;
}

inline int32_t ConnectionService::getMaxConnectionCount()
{
   return mMaxConnections;
}

inline int32_t ConnectionService::getConnectionCount()
{
   return mCurrentConnections;
}
