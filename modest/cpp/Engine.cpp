/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Engine.h"
#include "OperationExecutor.h"

using namespace db::modest;

Engine::Engine()
{
   mState = new State();
   mOpDispatcher = new OperationDispatcher(this);
}

Engine::~Engine()
{
   // ensure engine is stopped
   stop();
   
   delete mOpDispatcher;
   delete mState;
}

void Engine::queue(Operation* op)
{
   // create an OperationExecutor
   OperationExecutor* e = new OperationExecutor(mState, op, mOpDispatcher);
   mOpDispatcher->queueOperation(e);
}

void Engine::start()
{
   mOpDispatcher->startDispatching();
}

void Engine::stop()
{
   mOpDispatcher->stopDispatching();
   mOpDispatcher->terminateRunningOperations();
}

ImmutableState* Engine::getState()
{
   return (ImmutableState*)mState;
}
