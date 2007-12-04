/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_OperationGuardChain_H
#define db_modest_OperationGuardChain_H

#include "db/modest/OperationGuard.h"

namespace db
{
namespace modest
{

/**
 * An OperationGuardChain is used to chain two OperationGuards together.
 * 
 * @author Dave Longley
 */
class OperationGuardChain : public OperationGuard
{
protected:
   /**
    * The OperationGuard to run first.
    */
   OperationGuard* mGuard1;
   CollectableOperationGuard mGuardReference1;
   
   /**
    * The OperationGuard to run second.
    */
   OperationGuard* mGuard2;
   CollectableOperationGuard mGuardReference2;
   
public:
   /**
    * Creates a new OperationGuardChain that chains the passed two
    * OperationGuards together in the order specified.
    * 
    * @param g1 the first OperationGuard.
    * @param g2 the second OperationGuard.
    */
   OperationGuardChain(OperationGuard* g1, OperationGuard* g2);
   OperationGuardChain(CollectableOperationGuard& g1, OperationGuard* g2);
   OperationGuardChain(OperationGuard* g1, CollectableOperationGuard& g2);
   OperationGuardChain(
      CollectableOperationGuard& g1, CollectableOperationGuard& g2);
   
   /**
    * Destructs this OperationGuardChain.
    */
   virtual ~OperationGuardChain();
   
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
   virtual bool canExecuteOperation(ImmutableState* s);
   
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
   virtual bool mustCancelOperation(ImmutableState* s);
};

} // end namespace modest
} // end namespace db
#endif
