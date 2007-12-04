/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/Operation.h"

using namespace db::modest;
using namespace db::rt;

Operation::Operation(Runnable& r) :
   Collectable<OperationImpl>(new OperationImpl(r))
{
}

Operation::Operation(CollectableRunnable& r) :
   Collectable<OperationImpl>(new OperationImpl(r))
{
}

Operation::Operation(OperationImpl* impl) :
   Collectable<OperationImpl>(impl)
{
}

Operation::~Operation()
{
}
