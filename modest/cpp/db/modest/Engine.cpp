/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
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
   mLock.lock();
   {
      mOpDispatcher->startDispatching();
   }
   mLock.unlock();
}

void Engine::stop()
{
   mLock.lock();
   {
      mOpDispatcher->stopDispatching();
      mOpDispatcher->terminateRunningOperations();
   }
   mLock.unlock();
}

inline ImmutableState* Engine::getState()
{
   return (ImmutableState*)mState;
}

inline ThreadPool* Engine::getThreadPool()
{
   return mOpDispatcher->getThreadPool();
}

inline OperationDispatcher* Engine::getOperationDispatcher()
{
   return mOpDispatcher;
}
