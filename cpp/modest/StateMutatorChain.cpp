/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/modest/StateMutatorChain.h"

using namespace monarch::modest;

StateMutatorChain::StateMutatorChain(
   StateMutator* g1, StateMutator* g2)
{
   mMutator1 = g1;
   mMutator2 = g2;
}

StateMutatorChain::StateMutatorChain(
   StateMutatorRef& g1, StateMutator* g2) :
   mMutatorReference1(g1)
{
   mMutator1 = &(*g1);
   mMutator2 = g2;
}

StateMutatorChain::StateMutatorChain(
   StateMutator* g1, StateMutatorRef& g2) :
   mMutatorReference2(g2)
{
   mMutator1 = g1;
   mMutator2 = &(*g2);
}

StateMutatorChain::StateMutatorChain(
   StateMutatorRef& g1, StateMutatorRef& g2) :
   mMutatorReference1(g1), mMutatorReference2(g2)
{
   mMutator1 = &(*g1);
   mMutator2 = &(*g2);
}

StateMutatorChain::~StateMutatorChain()
{
}

void StateMutatorChain::mutatePreExecutionState(Operation& op)
{
   mMutator1->mutatePreExecutionState(op);
   if(mMutator2 != NULL)
   {
      mMutator2->mutatePreExecutionState(op);
   }
}

void StateMutatorChain::mutatePostExecutionState(Operation& op)
{
   mMutator1->mutatePostExecutionState(op);
   if(mMutator2 != NULL)
   {
      mMutator2->mutatePostExecutionState(op);
   }
}
