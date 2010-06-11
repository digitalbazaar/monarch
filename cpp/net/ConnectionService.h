/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_net_ConnectionService_H
#define monarch_net_ConnectionService_H

#include "monarch/modest/OperationList.h"
#include "monarch/net/Connection.h"
#include "monarch/net/PortService.h"
#include "monarch/net/SocketDataPresenter.h"

namespace monarch
{
namespace net
{

// forward declare classes
class Server;
class ConnectionServicer;

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
public monarch::modest::OperationGuard,
public monarch::modest::StateMutator
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
    * The maximum number of connections for this service.
    */
   int32_t mMaxConnections;

   /**
    * The current number of connections for this service.
    */
   int32_t mCurrentConnections;

   /**
    * The number of connections to backlog.
    */
   int mBacklog;

   /**
    * A list of Operations running ConnectionServicers.
    */
   monarch::modest::OperationList mRunningServicers;

public:
   /**
    * Creates a new ConnectionService for a Server.
    *
    * @param server the Server this service is for.
    * @param address the address to listen for Connections on.
    * @param servicer the ConnectionServicer to service Connections with.
    * @param presenter the SocketDataPresenter to present data with.
    * @param name a name for this service.
    */
   ConnectionService(
      Server* server,
      InternetAddress* address,
      ConnectionServicer* servicer,
      SocketDataPresenter* presenter,
      const char* name = "unnamed");

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
      monarch::modest::ImmutableState* s, monarch::modest::Operation& op);

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
      monarch::modest::ImmutableState* s, monarch::modest::Operation& op);

   /**
    * Alters the passed State directly before an Operation executes.
    *
    * @param s the State to alter.
    * @param op the Operation to be executed.
    */
   virtual void mutatePreExecutionState(
      monarch::modest::State* s, monarch::modest::Operation& op);

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
      monarch::modest::State* s, monarch::modest::Operation& op);

   /**
    * Runs this ConnectionService.
    */
   virtual void run();

   /**
    * Services a Connection and then closes it.
    *
    * @param op the Operation servicing the connection with socket as user data.
    */
   virtual void serviceConnection(void* operation);

   /**
    * Sets the maximum number of concurrent connections this service should
    * allow.
    *
    * @param count the maximum number of concurrent connections this service
    *        should allow.
    */
   virtual void setMaxConnectionCount(int32_t count);

   /**
    * Gets the maximum number of concurrent connections this service allows.
    *
    * @return the maximum number of concurrent connections this service allows.
    */
   virtual int32_t getMaxConnectionCount();

   /**
    * Gets the current number of connections being serviced.
    *
    * @return the current number of connections being serviced.
    */
   virtual int32_t getConnectionCount();

   /**
    * Sets the number of connections to backlog. Must be set before starting
    * the PortService.
    *
    * @param backlog the number of connections to backlog.
    */
   virtual void setBacklog(int backlog);

   /**
    * Gets the number of connections to backlog.
    *
    * @return the number of connections to backlog.
    */
   virtual int getBacklog();

protected:
   /**
    * Initializes this service and creates the Operation for running it,
    * typically through the Server's OperationRunner. If the service could
    * not be initialized, an exception should be set on the current thread
    * indicating the reason why the service could not be initialized.
    *
    * @return the Operation for running this service, or NULL if the
    *         service could not be initialized.
    */
   virtual monarch::modest::Operation initialize();

   /**
    * Called to clean up resources for this service that were created or
    * obtained via a call to initialize(). If there are no resources to
    * clean up, then this method should have no effect.
    */
   virtual void cleanup();
};

} // end namespace net
} // end namespace monarch
#endif
