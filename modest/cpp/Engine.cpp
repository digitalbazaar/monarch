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
   
   // clear queued operations
   mOpDispatcher->clearQueuedOperations();
   
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
#include <iostream>
void Engine::stop()
{
   std::cout << "Engine: STOPPING DISPATCH" << std::endl;
   mOpDispatcher->stopDispatching();
   std::cout << "Engine: TERMINATING ALL RUNNING" << std::endl;
   mOpDispatcher->terminateRunningOperations();
   std::cout << "Engine: ALL RUNNING TERMINATED." << std::endl;
}

ImmutableState* Engine::getState()
{
   return (ImmutableState*)mState;
}
