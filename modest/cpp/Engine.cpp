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
   lock();
   {
      // create an OperationExecutor
      OperationExecutor* e = new OperationExecutor(mState, op, mOpDispatcher);
      mOpDispatcher->queueOperation(e);
   }
   unlock();
}

void Engine::start()
{
   lock();
   {
      mOpDispatcher->startDispatching();
   }
   unlock();
}

void Engine::stop()
{
   lock();
   {
      mOpDispatcher->stopDispatching();
      mOpDispatcher->terminateRunningOperations();
   }
   unlock();
}

ImmutableState* Engine::getState()
{
   return (ImmutableState*)mState;
}
