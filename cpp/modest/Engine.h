/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_modest_Engine_H
#define monarch_modest_Engine_H

#include "monarch/modest/Operation.h"
#include "monarch/rt/JobDispatcher.h"
#include "monarch/rt/ThreadPool.h"

namespace monarch
{
namespace modest
{

/**
 * A Modest Engine (MODular Extensible State Engine) is a lightweight
 * processing engine that modifies state by using Operations. Multiple
 * Operations may be executed concurrently. An Engine can be shared amongst
 * multiple Modules that each provide unique functionality.
 *
 * The Modest Engine is intended to be "modest" in its complexity and
 * code base, but powerful in its extensibility. The core engine provides
 * a cross-platform thread pool for executing Operations in an orderly
 * fashion. The design intends to allow developers to create Modules that
 * can concurrently run multiple Operations that must by synchronized with
 * one another in some fashion.
 *
 * Operations may be queued with the Engine to be dispatched. Before
 * any Operation can be dispatched for execution, any associated state
 * must be checked against the Operation's guard for compatibility.
 *
 * @author Dave Longley
 */
class Engine : protected monarch::rt::JobDispatcher
{
protected:
   /**
    * The set to true when a dispatch should occur. This is true to begin with
    * and is set to true when a new operation is queued or executed, or when
    * one expires.
    */
   bool mDispatch;

   /**
    * A lock for starting/stopping the engine.
    */
   monarch::rt::ExclusiveLock mStartLock;

   /**
    * A lock for manipulating state.
    */
   monarch::rt::ExclusiveLock mStateLock;

public:
   /**
    * Creates a new Engine.
    */
   Engine();

   /**
    * Destructs this Engine.
    */
   virtual ~Engine();

   /**
    * Starts this Engine. This will begin executing queued Operations.
    */
   virtual void start();

   /**
    * Stops this Engine. This will stop executing queued Operations, clear
    * the queue, and terminate all currently running Operations.
    */
   virtual void stop();

   /**
    * Queues the passed Operation for execution. The Operation may fail to
    * execute if the current state is not compatible with the Operation's
    * guard. The Operation may also be placed in a wait queue to be checked
    * later for execution.
    *
    * After this method has been called, the Operation may be waited on until
    * it finishes or is canceled, by calling op->waitFor().
    *
    * @param op the Operation to execute.
    */
   virtual void queue(Operation& op);

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
    * Gets the current thread's Operation. This method assumes that you
    * know that the current thread has an Operation. Do not call it if
    * you aren't certain of this, it may result in memory corruption.
    *
    * @return the current thread's Operation, NULL if the Operation is not
    *         registered with this dispatcher.
    */
   virtual Operation getCurrentOperation();

   /**
    * Gets this Engine's ThreadPool.
    *
    * @return this Engine's ThreadPool.
    */
   virtual monarch::rt::ThreadPool* getThreadPool();

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

protected:
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

   /**
    * Runs an operation.
    *
    * @param op the operation to run.
    */
   virtual void runOperation(Operation* op);

   /**
    * Frees an operation.
    *
    * @param op the operation to free.
    */
   virtual void freeOperation(Operation* op);
};

} // end namespace modest
} // end namespace monarch
#endif
