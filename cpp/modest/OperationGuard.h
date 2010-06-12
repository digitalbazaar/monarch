/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_modest_OperationGuard_H
#define monarch_modest_OperationGuard_H

#include "monarch/rt/Collectable.h"

namespace monarch
{
namespace modest
{

// forward declare Operation
class Operation;

/**
 * An OperationGuard defines a set of conditions that the current state must
 * meet in order for an Engine to be able to execute an Operation.
 *
 * The state to inspect should be accessible by the class that implements this
 * interface or made accessible via the user data attribute of the Operation.
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
    * This method allows for custom conditions to be checked that allow
    * this Operation to execute after it has been queued, or that require
    * it to wait or be canceled.
    *
    * It returns true if the current state meets the conditions of this guard
    * such that an Operation that requires this guard could be executed
    * immediately by an Engine.
    *
    * This method will be called before an Operation is executed. It should
    * only return true if the Operation can execute immediately. If the
    * Operation should wait or perhaps be canceled, then this method
    * should return false. If this method returns true, then Operation will
    * be executed immediately, if it returns false, then this guard's
    * "mustCancelOperation()" method will be called.
    *
    * @param op the Operation this guard is for.
    *
    * @return true if the state indicates an Engine could immediately execute
    *         an Operation with this guard, false if it should wait and be
    *         checked for possible cancelation.
    */
   virtual bool canExecuteOperation(Operation& op) = 0;

   /**
    * This method allows for custom conditions to be checked that require
    * this Operation to be canceled after it has been queued for execution,
    * but before it is executed.
    *
    * It returns true if the current state cannot meet the conditions of this
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
    * @param op the Operation this guard is for.
    *
    * @return true if the state indicates an Engine must cancel an Operation
    *         with this guard before it executes, false if not.
    */
   virtual bool mustCancelOperation(Operation &op) = 0;
};

// define a reference counted OperationGuard type
typedef monarch::rt::Collectable<OperationGuard> OperationGuardRef;

} // end namespace modest
} // end namespace monarch
#endif
