/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/modest/OperationGuardChain.h"

#include "monarch/modest/Operation.h"

using namespace monarch::modest;

OperationGuardChain::OperationGuardChain(
   OperationGuard* g1, OperationGuard* g2)
{
   mGuard1 = g1;
   mGuard2 = g2;
}

OperationGuardChain::OperationGuardChain(
   OperationGuardRef& g1, OperationGuard* g2) :
   mGuardReference1(g1)
{
   mGuard1 = &(*g1);
   mGuard2 = g2;
}

OperationGuardChain::OperationGuardChain(
   OperationGuard* g1, OperationGuardRef& g2) :
   mGuardReference2(g2)
{
   mGuard1 = g1;
   mGuard2 = &(*g2);
}

OperationGuardChain::OperationGuardChain(
   OperationGuardRef& g1, OperationGuardRef& g2) :
   mGuardReference1(g1), mGuardReference2(g2)
{
   mGuard1 = &(*g1);
   mGuard2 = &(*g2);
}

OperationGuardChain::~OperationGuardChain()
{
}

bool OperationGuardChain::canExecuteOperation(ImmutableState* s, Operation &op)
{
   return
      mGuard1->canExecuteOperation(s, op) &&
      (mGuard2 == NULL || mGuard2->canExecuteOperation(s, op));
}

bool OperationGuardChain::mustCancelOperation(ImmutableState* s, Operation &op)
{
   return
      mGuard1->mustCancelOperation(s, op) ||
      (mGuard2 != NULL && mGuard2->canExecuteOperation(s, op));
}
