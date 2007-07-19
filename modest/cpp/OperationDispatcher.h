/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_OperationDispatcher_H
#define db_modest_OperationDispatcher_H

#include "JobDispatcher.h"
#include "Operation.h"

namespace db
{
namespace modest
{

// forward declare OperationExecutor
class OperationExecutor;

/**
 * An OperationDispatcher is used to dispatch Operations for execution. Before
 * any Operation can be dispatched for execution, the associated Engine's
 * State must be checked against the Operation's guard for compatibility.
 * 
 * @author Dave Longley
 */
class OperationDispatcher : protected db::rt::JobDispatcher
{
protected:
   /**
    * The Engine this OperationDispatcher dispatches Operations to.
    */
   Engine* mEngine;
   
   /**
    * The set to true when a dispatch should occur. This is true to begin with
    * and is set to true when a new operation is queued or executed, or when
    * one expires.
    */
   bool mDispatch;
   
   /**
    * A list of expired OperationExecutors to clean up.
    */
   std::list<OperationExecutor*> mExpiredExecutors;
   
   /**
    * Returns true if this dispatcher has a job it can dispatch.
    * 
    * @return true if this dispatcher has a job it can dispatch.
    */
   virtual bool canDispatch();
   
   /**
    * Dispatches the next Operation available.
    */
   virtual void dispatchNextJob();
   
   /**
    * Cleans up any expired executors.
    */
   virtual void cleanupExpiredExecutors();
   
public:
   /**
    * Creates a new OperationDispatcher for the given Engine.
    * 
    * @param e the Engine to dispatch Operations to.
    */
   OperationDispatcher(Engine* e);
   
   /**
    * Destructs this OperationDispatcher.
    */
   virtual ~OperationDispatcher();
   
   /**
    * Queues an Operation for execution.
    * 
    * @param e the OperationExecutor to queue to execute an Operation.
    */
   virtual void queueOperation(OperationExecutor* e);
   
   /**
    * Starts dispatching Operations.
    */
   virtual void startDispatching();
   
   /**
    * Stops dispatching Operations. This does not terminate the Operations
    * that are already running.
    */
   virtual void stopDispatching();
   
   /**
    * Clears all queued Operations.
    */
   virtual void clearQueuedOperations(); 
   
   /**
    * Interrupts and joins all running Operations. Queued Operations are not
    * affected.
    */
   virtual void terminateRunningOperations();
   
   /**
    * Adds an expired OperationExecutor to the list of expired executors for
    * clean up.
    * 
    * @param e the OperationExecutor to add.
    */
   virtual void addExpiredExecutor(OperationExecutor* e);
   
   /**
    * Gets the thread pool for running Operations.
    * 
    * @return the therad pool for running Operations.
    */
   virtual db::rt::JobThreadPool* getThreadPool();
   
   /**
    * Gets the number of Operations that are in the queue to be executed.
    * 
    * @return the number of Operations that are queued to be executed.
    */
   virtual unsigned int getQueuedOperationCount();
   
   /**
    * Gets the number of Operations that are in the queue and that are
    * running.
    * 
    * @return the number of Operations that are queued to be executed
    *         plus the Operations that are already running.
    */
   virtual unsigned int getTotalOperationCount();
};

} // end namespace modest
} // end namespace db
#endif
