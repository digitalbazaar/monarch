/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_modest_StateMutator_H
#define monarch_modest_StateMutator_H

#include "monarch/rt/Collectable.h"

namespace monarch
{
namespace modest
{

// forward declare Operation
class Operation;

/**
 * A StateMutator is an interface used by an Operation to alter the current
 * state directly before and/or after the Operation's execution.
 *
 * The state to alter should be accessible by the class that implements this
 * interface or made accessible via the user data attribute of the Operation.
 *
 * @author Dave Longley
 */
class StateMutator
{
public:
   /**
    * Creates a new StateMutator.
    */
   StateMutator() {};

   /**
    * Destructs this StateMutator.
    */
   virtual ~StateMutator() {};

   /**
    * Alters the current state directly before an Operation executes.
    *
    * @param op the Operation to be executed.
    */
   virtual void mutatePreExecutionState(Operation& op) {};

   /**
    * Alters the current state directly after an Operation finishes or
    * was canceled.
    *
    * The passed Operation may be checked to see if it finished or was
    * canceled, etc.
    *
    * @param op the Operation that finished or was canceled.
    */
   virtual void mutatePostExecutionState(Operation& op) {};
};

// define a reference counted StateMutator type
typedef monarch::rt::Collectable<StateMutator> StateMutatorRef;

} // end namespace modest
} // end namespace monarch
#endif
