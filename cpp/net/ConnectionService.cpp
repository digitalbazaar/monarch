/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/net/ConnectionService.h"

#include "db/logging/Logging.h"
#include "db/net/Server.h"
#include "db/net/TcpSocket.h"
#include "db/net/Internet6Address.h"
#include "db/rt/RunnableDelegate.h"

using namespace std;
using namespace db::modest;
using namespace db::net;
using namespace db::rt;

ConnectionService::ConnectionService(
   Server* server,
   InternetAddress* address,
   ConnectionServicer* servicer,
   SocketDataPresenter* presenter,
   const char* name) :
   PortService(server, address, name)
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
      rval->setUserData(&mSocket);
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

   if(op->getUserData() == mSocket)
   {
      // accept OP can execute if server is running
      rval = mServer->isRunning();
   }
   else if(mServer->isRunning())
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
   bool rval = false;

   // must cancel any OP if the server is not running
   if(!mServer->isRunning())
   {
      rval = true;

      // operation's user data is the socket being serviced, either it is
      // the socket accepting connections or a socket that is servicing one
      Socket* socket = static_cast<Socket*>(op->getUserData());

      // if the socket isn't the accept socket, then it needs to be cleaned up
      if(socket != mSocket)
      {
         // close and clean up the operation's socket
         socket->close();
         delete socket;
         op->setUserData(NULL);
      }
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
         // create RunnableDelegate to service connection
         // and run it as an Operation
         RunnableRef r =
            new RunnableDelegate<ConnectionService>(
               this, &ConnectionService::serviceConnection, s);
         Operation op(r);
         op->setUserData(s);
         op->addGuard(this);
         op->addStateMutator(this);
         mRunningServicers.add(op);

         // run operation
         mServer->getOperationRunner()->runOperation(op);
      }
   }

   // close socket
   mSocket->close();

   // terminate running servicers
   mRunningServicers.terminate();
}

void ConnectionService::serviceConnection(void* s)
{
   // ensure the Socket can be wrapped with at least standard data presentation
   bool secure = false;
   Socket* socket = static_cast<Socket*>(s);
   Socket* wrapper = socket;
   if(mDataPresenter != NULL)
   {
      // the secure flag will be set by the data presenter
      wrapper = mDataPresenter->createPresentationWrapper(socket, secure);
   }

   if(wrapper != NULL)
   {
      // create connection
      Connection* c = new Connection(wrapper, true);
      c->setSecure(secure);

      // get local/remote addresses
      SocketAddress* local = c->getLocalAddress();
      SocketAddress* remote = c->getRemoteAddress();

      // log connection
      DB_CAT_DEBUG(DB_NET_CAT, "%s:%i servicing %s connection from %s:%i",
         local->getAddress(),
         local->getPort(),
         secure ? "secure" : "non-secure",
         remote->getAddress(),
         remote->getPort());

      // service connection
      mServicer->serviceConnection(c);

      // log connection
      DB_CAT_DEBUG(DB_NET_CAT, "%s:%i serviced %s connection from %s:%i",
         local->getAddress(),
         local->getPort(),
         secure ? "secure" : "non-secure",
         remote->getAddress(),
         remote->getPort());

      // close and clean up connection
      c->close();
      delete c;
   }
   else
   {
      // close socket, data cannot be presented in standard format
      socket->close();
      delete socket;
   }
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
