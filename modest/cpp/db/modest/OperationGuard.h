/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_OperationGuard_H
#define db_modest_OperationGuard_H

#include "db/rt/Collectable.h"
#include "db/modest/ImmutableState.h"

namespace db
{
namespace modest
{

/**
 * An OperationGuard defines a set of conditions that a State must meet
 * in order for its Engine to be able to execute an Operation.
 * 
 * @author Dave Longley
 */
class OperationGuard
{
public:
   /**
    * Creates a new OperationGuard.
    */
   OperationGuard() {};
   
   /**
    * Destructs this OperationGuard.
    */
   virtual ~OperationGuard() {};
   
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
   virtual bool canExecuteOperation(ImmutableState* s) = 0;
   
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
   virtual bool mustCancelOperation(ImmutableState* s) = 0;
};

// define a collectable OperationGuard
typedef db::rt::Collectable<OperationGuard> CollectableOperationGuard;

} // end namespace modest
} // end namespace db
#endif
