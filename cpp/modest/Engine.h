/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_modest_Engine_H
#define monarch_modest_Engine_H

#include "monarch/modest/Operation.h"
#include "monarch/rt/ThreadPool.h"

namespace monarch
{
namespace modest
{

// forward declare OperationDispatcher
class OperationDispatcher;

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
 * @author Dave Longley
 */
class Engine
{
protected:
   /**
    * The OperationDispatcher for dispatching Operations.
    */
   OperationDispatcher* mOpDispatcher;

   /**
    * A lock for starting/stopping this Engine.
    */
   monarch::rt::ExclusiveLock mLock;

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
    * Starts this Engine. This will begin executing queued Operations.
    */
   virtual void start();

   /**
    * Stops this Engine. This will stop executing queued Operations and
    * interrupt all currently running Operations.
    */
   virtual void stop();

   /**
    * Gets this Engine's ThreadPool.
    *
    * @return this Engine's ThreadPool.
    */
   virtual monarch::rt::ThreadPool* getThreadPool();

   /**
    * Gets this Engine's OperationDispatcher.
    *
    * @return this Engine's OperationDispatcher.
    */
   virtual OperationDispatcher* getOperationDispatcher();
};

} // end namespace modest
} // end namespace monarch
#endif
