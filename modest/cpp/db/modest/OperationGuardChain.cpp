/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/OperationGuardChain.h"

using namespace db::modest;

OperationGuardChain::OperationGuardChain(
   OperationGuard* g1, OperationGuard* g2)
{
   mGuard1 = g1;
   mGuard2 = g2;
}

OperationGuardChain::OperationGuardChain(
   CollectableOperationGuard& g1, OperationGuard* g2) :
   mGuardReference1(g1)
{
   mGuard1 = &(*g1);
   mGuard2 = g2;
}

OperationGuardChain::OperationGuardChain(
   OperationGuard* g1, CollectableOperationGuard& g2) :
   mGuardReference2(g2)
{
   mGuard1 = g1;
   mGuard2 = &(*g2);
}

OperationGuardChain::OperationGuardChain(
   CollectableOperationGuard& g1, CollectableOperationGuard& g2) :
   mGuardReference1(g1), mGuardReference2(g2)
{
   mGuard1 = &(*g1);
   mGuard2 = &(*g2);
}

OperationGuardChain::~OperationGuardChain()
{
}

bool OperationGuardChain::canExecuteOperation(ImmutableState* s)
{
   return
      mGuard1->canExecuteOperation(s) &&
      (mGuard2 == NULL || mGuard2->canExecuteOperation(s));
}

bool OperationGuardChain::mustCancelOperation(ImmutableState* s)
{
   return
      mGuard1->mustCancelOperation(s) ||
      (mGuard2 != NULL && mGuard2->canExecuteOperation(s));
}
