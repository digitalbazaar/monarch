/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/Engine.h"
#include "db/modest/OperationDispatcher.h"

using namespace db::rt;
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

void Engine::queue(Operation& op)
{
   // queue the operation with the dispatcher
   mOpDispatcher->queueOperation(op);
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

ThreadPool* Engine::getThreadPool()
{
   return mOpDispatcher->getThreadPool();
}

OperationDispatcher* Engine::getOperationDispatcher()
{
   return mOpDispatcher;
}
