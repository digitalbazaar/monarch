/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef OperationEnvironment_H
#define OperationEnvironment_H

#include "ImmutableState.h"

namespace db
{
namespace modest
{

/**
 * An OperationEnvironment defines a set of conditions that a State must meet
 * in order for its Engine to be able to execute an Operation.
 * 
 * @author Dave Longley
 */
class OperationEnvironment
{
public:
   /**
    * Creates a new OperationEnvironment.
    */
   OperationEnvironment() {};
   
   /**
    * Destructs this OperationEnvironment.
    */
   virtual ~OperationEnvironment() {};
   
   /**
    * Returns true if the passed State meets the conditions of this environment
    * such that an Operation that requires this environment could be executed
    * immediately by an Engine with the given State.
    * 
    * @param s the ImmutableState to inspect.
    * 
    * @return true if an Engine with the given State could immediately execute
    *         an Operation with this environment.
    */
   virtual bool canExecuteOperation(ImmutableState* s) = 0;
   
   /**
    * Returns true if the passed State meets the conditions of this evironment
    * such that an Operation that requires this Environment could be queued to
    * be checked later for execution by an Engine with the given State.
    * 
    * @param s the ImmutableState to inspect.
    * 
    * @return true if an Engine with the given State could place an Operation
    *         with this environment in a wait queue to check it later for
    *         execution.
    */
   virtual bool canQueueOperation(ImmutableState* s) = 0;
};

} // end namespace modest
} // end namespace db
#endif
