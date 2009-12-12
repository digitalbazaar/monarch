/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_modest_OperationRunner_H
#define db_modest_OperationRunner_H

#include "monarch/rt/Runnable.h"
#include "monarch/modest/Operation.h"
#include "monarch/modest/OperationGuard.h"
#include "monarch/modest/StateMutator.h"

namespace db
{
namespace modest
{

/**
 * An OperationRunner is an interface that provides a method for running
 * Operations. Any passed Operation may have OperationGuards or StateMutators
 * added to it in order to provide additional logic that is specific to
 * the OperationRunner's implementation.
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
    * Queues the passed Operation with an appropriate modest engine for
    * execution. OperationGuards and/or StateMutators may be added to the
    * Operation.
    *
    * @param op the Operation to queue with a modest engine for execution.
    */
   virtual void runOperation(Operation& op) = 0;
};

} // end namespace modest
} // end namespace db
#endif
