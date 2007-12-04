/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_StateMutatorChain_H
#define db_modest_StateMutatorChain_H

#include "db/modest/StateMutator.h"

namespace db
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
   CollectableStateMutator mMutatorReference1;
   
   /**
    * The StateMutator to run second.
    */
   StateMutator* mMutator2;
   CollectableStateMutator mMutatorReference2;
   
public:
   /**
    * Creates a new StateMutatorChain that chains the passed two
    * StateMutators together in the order specified.
    * 
    * @param g1 the first StateMutator.
    * @param g2 the second StateMutator.
    */
   StateMutatorChain(StateMutator* g1, StateMutator* g2);
   StateMutatorChain(CollectableStateMutator& g1, StateMutator* g2);
   StateMutatorChain(StateMutator* g1, CollectableStateMutator& g2);
   StateMutatorChain(
      CollectableStateMutator& g1, CollectableStateMutator& g2);
   
   /**
    * Destructs this StateMutatorChain.
    */
   virtual ~StateMutatorChain();
   
   /**
    * Alters the passed State directly before an Operation executes.
    * 
    * @param s the State to alter.
    * @param op the Operation to be executed.
    */
   virtual void mutatePreExecutionState(State* s, Operation& op);
   
   /**
    * Alters the passed State directly after an Operation finishes or
    * was canceled.
    * 
    * The passed Operation may be checked to see if it finished or was
    * canceled, etc.
    * 
    * @param s the State to alter.
    * @param op the Operation that finished or was canceled.
    */
   virtual void mutatePostExecutionState(State* s, Operation& op);
};

} // end namespace modest
} // end namespace db
#endif
