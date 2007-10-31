/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_OperationRunner_H
#define db_modest_OperationRunner_H

#include "db/rt/Runnable.h"
#include "db/modest/Operation.h"
#include "db/modest/OperationGuard.h"
#include "db/modest/StateMutator.h"

namespace db
{
namespace modest
{

/**
 * An OperationRunner is an interface that provides methods for creating
 * and running Operations with a given Runnable, OperationGuard, and
 * StateMutator. The passed Runnable, OperationGuard, and/or StateMutator
 * may be wrapped by other classes to provide additional logic.
 * 
 * @author Dave Longley
 */
class OperationRunner
{
public:
   /**
    * Creates a new OperationRunner.
    */
   OperationRunner() {};
   
   /**
    * Destructs this OperationRunner.
    */
   virtual ~OperationRunner() {};
   
   /**
    * Creates an Operation from the given Runnable, OperationGuard,
    * and StateMutator that is to be run by this OperationRunner.
    * 
    * The passed Runnable, OperationGuard, and/or StateMutator may be
    * wrapped by other classes to provide additional logic.
    * 
    * @param r the Runnable with code to execute during the operation.
    * @param g the OperationGuard to use.
    * @param m the StateMutator to use.
    * 
    * @return the Operation object to use to monitor the status of the
    *         Operation.
    */
   virtual Operation createOperation(
      db::rt::Runnable* r, OperationGuard* g, StateMutator* m) = 0;
   
   /**
    * Queues the passed Operation with an appropriate modest engine for
    * execution.
    * 
    * @param op the Operation to queue with a modest engine for execution.
    */
   virtual void runOperation(Operation op) = 0;
};

} // end namespace modest
} // end namespace db
#endif
