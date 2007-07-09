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
    * Returns true if the passed State cannot meet the conditions of this
    * environment such that an Operation that requires this Environment must
    * be immediately canceled.
    * 
    * This method may return false when an Operation may not be able to be
    * executed immediately, but it needn't be canceled either, instead it
    * can be checked again later for execution or cancelation.
    * 
    * @param s the ImmutableState to inspect.
    * 
    * @return true if an Engine with the given State must cancel an Operation
    *         with this environment, false if not.
    */
   virtual bool mustCancelOperation(ImmutableState* s) = 0;
};

} // end namespace modest
} // end namespace db
#endif
