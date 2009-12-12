/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/modest/Operation.h"

using namespace monarch::modest;
using namespace monarch::rt;

Operation::Operation(Runnable& r) :
   Collectable<OperationImpl>(new OperationImpl(r))
{
}

Operation::Operation(RunnableRef& r) :
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
