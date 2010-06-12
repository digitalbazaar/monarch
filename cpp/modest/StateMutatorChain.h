/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_modest_StateMutatorChain_H
#define monarch_modest_StateMutatorChain_H

#include "monarch/modest/StateMutator.h"

namespace monarch
{
namespace modest
{

/**
 * An StateMutatorChain is used to chain two StateMutators together.
 *
 * @author Dave Longley
 */
class StateMutatorChain : public StateMutator
{
protected:
   /**
    * The StateMutator to run first.
    */
   StateMutator* mMutator1;
   StateMutatorRef mMutatorReference1;

   /**
    * The StateMutator to run second.
    */
   StateMutator* mMutator2;
   StateMutatorRef mMutatorReference2;

public:
   /**
    * Creates a new StateMutatorChain that chains the passed two
    * StateMutators together in the order specified.
    *
    * @param g1 the first StateMutator.
    * @param g2 the second StateMutator.
    */
   StateMutatorChain(StateMutator* g1, StateMutator* g2);
   StateMutatorChain(StateMutatorRef& g1, StateMutator* g2);
   StateMutatorChain(StateMutator* g1, StateMutatorRef& g2);
   StateMutatorChain(StateMutatorRef& g1, StateMutatorRef& g2);

   /**
    * Destructs this StateMutatorChain.
    */
   virtual ~StateMutatorChain();

   /**
    * Alters the current state directly before an Operation executes.
    *
    * @param op the Operation to be executed.
    */
   virtual void mutatePreExecutionState(Operation& op);

   /**
    * Alters the current state directly after an Operation finishes or
    * was canceled.
    *
    * The passed Operation may be checked to see if it finished or was
    * canceled, etc.
    *
    * @param op the Operation that finished or was canceled.
    */
   virtual void mutatePostExecutionState(Operation& op);
};

} // end namespace modest
} // end namespace monarch
#endif
