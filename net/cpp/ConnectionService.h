/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_ConnectionService_H
#define db_net_ConnectionService_H

#include "InternetAddress.h"
#include "OperationGuard.h"
#include "StateMutator.h"
#include "Runnable.h"
#include "SocketDataPresenter.h"
#include "OperationList.h"
#include "ConnectionServicer.h"
#include "ConnectionWorker.h"

namespace db
{
namespace net
{

// forward declare Server
class Server;

/**
 * A ConnectionService listens for incoming Socket connections on a given
 * address and accepts them.
 * 
 * When a connection is accepted, a SocketDataPresenter is used to create a
 * SocketWrapper to handle presenting the Socket data in a standard fashion.
 * Then a Connection is created and passed off to be serviced by a
 * ConnectionServicer.
 * 
 * @author Dave Longley
 */
class ConnectionService :
public virtual db::rt::Object,
public db::modest::OperationGuard,
public db::modest::StateMutator,
public db::rt::Runnable
{
protected:
   /**
    * The Server associated with this ConnectionService.
    */
   Server* mServer;
   
   /**
    * The address to listen for Connections on.
    */
   InternetAddress* mAddress;
   
   /**
    * The ConnectionServicer for this service.
    */
   ConnectionServicer* mServicer;
   
   /**
    * The SocketDataPresenter to use to present data in a standard format.
    */
   SocketDataPresenter* mDataPresenter;
   
   /**
    * A list of Operations running ConnectionServicers.
    */
   db::modest::OperationList mRunningServicers;
   
   /**
    * A list of ConnectionWorkers.
    */
   std::list<ConnectionWorker*> mWorkers;
   
   /**
    * The maximum number of connections to handle.
    */
   unsigned int mMaxConnectionCount;
   
   /**
    * The current number of connections being handled.
    */
   unsigned int mConnectionCount;
   
   /**
    * Cleans up any expired ConnectionWorkers.
    */
   virtual void cleanupWorkers();
   
public:
   /**
    * Creates a new ConnectionService for a Server.
    * 
    * @param server the Server this service is for.
    * @param address the address to listen for Connections on.
    * @param servicer the ConnectionServicer to service Connections with.
    * @param presenter the SocketDataPresenter to present data with.
    */
   ConnectionService(
      Server* server,
      InternetAddress* address,
      ConnectionServicer* servicer,
      SocketDataPresenter* presenter);
   
   /**
    * Destructs this ConnectionService.
    */
   virtual ~ConnectionService();
   
   /**
    * Returns true if the passed State meets the conditions of this guard
    * such that an Operation that requires this guard could be executed
    * immediately by an Engine with the given State.
    * 
    * @param s the ImmutableState to inspect.
    * 
    * @return true if an Engine with the given State could immediately execute
    *         an Operation with this guard.
    */
   virtual bool canExecuteOperation(db::modest::ImmutableState* s);
   
   /**
    * Returns true if the passed State cannot meet the conditions of this
    * guard such that an Operation that requires this guard must be
    * immediately canceled.
    * 
    * This method may return false when an Operation may not be able to be
    * executed immediately, but it needn't be canceled either, instead it
    * can be checked again later for execution or cancelation.
    * 
    * @param s the ImmutableState to inspect.
    * 
    * @return true if an Engine with the given State must cancel an Operation
    *         with this guard, false if not.
    */
   virtual bool mustCancelOperation(db::modest::ImmutableState* s);
   
   /**
    * Alters the passed State directly before an Operation executes.
    * 
    * @param s the State to alter.
    * @param op the Operation to be executed.
    */
   virtual void mutatePreExecutionState(
      db::modest::State* s, db::modest::Operation* op);
   
   /**
    * Alters the passed State directly after an Operation finishes or
    * was canceled.
    * 
    * The passed Operation may be checked to see if it finished or was
    * canceled, etc.
    * 
    * @param s the State to alter.
    * @param op the Operation that finished or was canceled.
    */
   virtual void mutatePostExecutionState(
      db::modest::State* s, db::modest::Operation* op);
   
   /**
    * Runs this ConnectionService.
    */
   virtual void run();
   
   /**
    * Creates a Connection from the given connected Socket and creates
    * a ConnectionWorker for it.
    * 
    * @param s the connected Socket.
    */
   virtual void createConnection(Socket* s);
   
   /**
    * Services the passed Connection and then closes it.
    * 
    * @param c the Connection to service.
    */
   virtual void serviceConnection(Connection* c);
   
   /**
    * Sets the maximum number of concurrent connections this service should
    * allow.
    * 
    * @param count the maximum number of concurrent connections this service
    *        should allow.
    */
   virtual void setMaxConnectionCount(unsigned int count);
   
   /**
    * Gets the maximum number of concurrent connections this service allows.
    * 
    * @return the maximum number of concurrent connections this service allows.
    */
   virtual unsigned int getMaxConnectionCount();
   
   /**
    * Gets the current number of connections being serviced.
    * 
    * @return the current number of connections being serviced.
    */
   virtual unsigned int getConnectionCount();
   
   /**
    * Gets the address for this ConnectionService.
    * 
    * @return the address for this ConnectionService.
    */
   virtual InternetAddress* getAddress();
};

} // end namespace net
} // end namespace db
#endif
