/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_OperationDispatcher_H
#define db_modest_OperationDispatcher_H

#include "db/rt/JobDispatcher.h"
#include "db/modest/Operation.h"
#include "db/modest/Engine.h"

namespace db
{
namespace modest
{

/**
 * An OperationDispatcher is used to dispatch Operations for execution. Before
 * any Operation can be dispatched for execution, the associated Engine's
 * State must be checked against the Operation's guard for compatibility.
 * 
 * @author Dave Longley
 */
class OperationDispatcher :
protected db::rt::JobThreadPool,
protected db::rt::JobDispatcher
{
protected:
   /**
    * The Engine this OperationDispatcher dispatches Operations to.
    */
   Engine* mEngine;
   
   /**
    * A map of OperationImpl's to Operation references.
    */
   typedef std::map<OperationImpl*, Operation> OperationMap;
   OperationMap mOpMap;
   
   /**
    * The set to true when a dispatch should occur. This is true to begin with
    * and is set to true when a new operation is queued or executed, or when
    * one expires.
    */
   bool mDispatch;
   
   /**
    * Returns true if this dispatcher has a job it can dispatch.
    * 
    * @return true if this dispatcher has a job it can dispatch.
    */
   virtual bool canDispatch();
   
   /**
    * Dispatches the Operations that can be dispatched.
    */
   virtual void dispatchJobs();
   
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
    * @param op the Operation to queue.
    */
   virtual void queueOperation(Operation& op);
   
   /**
    * Starts dispatching Operations.
    */
   using JobDispatcher::startDispatching;
   
   /**
    * Stops dispatching Operations. This does not terminate the Operations
    * that are already running.
    */
   using JobDispatcher::stopDispatching;
   
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
    * Called by a JobThread when it completes its job.
    * 
    * @param t the JobThread that completed its job.
    */
   virtual void jobCompleted(db::rt::JobThread* t);
   
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
