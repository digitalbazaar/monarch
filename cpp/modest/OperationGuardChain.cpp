/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/modest/OperationGuardChain.h"

#include "monarch/modest/Operation.h"

using namespace monarch::modest;

OperationGuardChain::OperationGuardChain(
   OperationGuard* g1, OperationGuard* g2) :
   mGuard1(g1),
   mGuard2(g2)
{
}

OperationGuardChain::OperationGuardChain(
   OperationGuardRef& g1, OperationGuard* g2) :
   mGuard1(&(*g1)),
   mGuardReference1(g1),
   mGuard2(g2)
{
}

OperationGuardChain::OperationGuardChain(
   OperationGuard* g1, OperationGuardRef& g2) :
   mGuard1(g1),
   mGuard2(&(*g2)),
   mGuardReference2(g2)
{
}

OperationGuardChain::OperationGuardChain(
   OperationGuardRef& g1, OperationGuardRef& g2) :
   mGuard1(&(*g1)),
   mGuardReference1(g1),
   mGuard2(&(*g2)),
   mGuardReference2(g2)
{
}

OperationGuardChain::~OperationGuardChain()
{
}

bool OperationGuardChain::canExecuteOperation(Operation &op)
{
   return
      mGuard1->canExecuteOperation(op) &&
      (mGuard2 == NULL || mGuard2->canExecuteOperation(op));
}

bool OperationGuardChain::mustCancelOperation(Operation &op)
{
   return
      mGuard1->mustCancelOperation(op) ||
      (mGuard2 != NULL && mGuard2->canExecuteOperation(op));
}
