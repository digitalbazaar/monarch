/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_ConnectionService_H
#define db_net_ConnectionService_H

#include "db/rt/Semaphore.h"
#include "db/net/PortService.h"
#include "db/net/Server.h"
#include "db/net/ConnectionServicer.h"
#include "db/net/SocketDataPresenter.h"
#include "db/modest/OperationList.h"
#include "db/net/ConnectionWorker.h"

namespace db
{
namespace net
{

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
public PortService,
public db::modest::OperationGuard
{
protected:
   /**
    * The ConnectionServicer for this service.
    */
   ConnectionServicer* mServicer;
   
   /**
    * The SocketDataPresenter to use to present data in a standard format.
    */
   SocketDataPresenter* mDataPresenter;
   
   /**
    * The Socket for this service.
    */
   Socket* mSocket;
   
   /**
    * The connection semaphore for this service.
    */
   db::rt::Semaphore mConnectionSemaphore;
   
   /**
    * A list of Operations running ConnectionServicers.
    */
   db::modest::OperationList mRunningServicers;
   
   /**
    * The current number of connections being handled.
    */
   unsigned int mConnectionCount;
   
   /**
    * Initializes this service and creates the Operation for running it,
    * typically through the Server's OperationRunner. If the service could
    * not be initialized, an exception should be set on the current thread
    * indicating the reason why the service could not be initialized.
    * 
    * @return the Operation for running this service, or NULL if the
    *         service could not be initialized.
    */
   virtual db::modest::Operation initialize();
   
   /**
    * Called to clean up resources for this service that were created or
    * obtained via a call to initialize(). If there are no resources to
    * clean up, then this method should have no effect.
    */
   virtual void cleanup();
   
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
    * This method allows for custom conditions to be checked that allow
    * this Operation to execute after it has been queued, or that require
    * it to wait or be canceled.
    * 
    * It returns true if the passed State meets the conditions of this guard
    * such that an Operation that requires this guard could be executed
    * immediately by an Engine with the given State.
    * 
    * This method will be called before an Operation is executed. It should
    * only return true if the Operation can execute immediately. If the
    * Operation should wait or perhaps be canceled, then this method
    * should return false. If this method returns true, then Operation will
    * be executed immediately, if it returns false, then this guard's
    * "mustCancelOperation()" method will be called.
    * 
    * @param s the ImmutableState to inspect.
    * @param op the Operation this guard is for.
    * 
    * @return true if an Engine with the given State could immediately execute
    *         an Operation with this guard, false if it should wait and be
    *         checked for possible cancelation.
    */
   virtual bool canExecuteOperation(
      db::modest::ImmutableState* s, db::modest::Operation& op);
   
   /**
    * This method allows for custom conditions to be checked that require
    * this Operation to be canceled after it has been queued for execution,
    * but before it is executed.
    * 
    * It returns true if the passed State cannot meet the conditions of this
    * guard such that an Operation that requires this guard must be immediately
    * canceled. 
    * 
    * This method may return false when an Operation may not be able to be
    * executed immediately, but it needn't be canceled either, instead it
    * can be checked again later for execution or cancelation.
    * 
    * It will be called before an Operation is executed, and after
    * "canExecuteOperation()" if that method returns false. This method
    * should only return true if the Operation should not be executed at
    * all and should be canceled instead.
    * 
    * To cancel any Operation after it has been queued or after it has started
    * executing, regardless of the Operation's custom cancel implementation,
    * use Operation.interrupt().
    * 
    * @param s the ImmutableState to inspect.
    * @param op the Operation this guard is for.
    * 
    * @return true if an Engine with the given State must cancel an Operation
    *         with this guard before it executes, false if not.
    */
   virtual bool mustCancelOperation(
      db::modest::ImmutableState* s, db::modest::Operation& op);
   
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
};

} // end namespace net
} // end namespace db
#endif
