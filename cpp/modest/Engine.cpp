/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/modest/Engine.h"

#include "monarch/modest/OperationDispatcher.h"

using namespace monarch::rt;
using namespace monarch::modest;

Engine::Engine()
{
   mOpDispatcher = new OperationDispatcher(this);
}

Engine::~Engine()
{
   // ensure engine is stopped
   stop();

   // clear queued operations
   mOpDispatcher->clearQueuedOperations();

   delete mOpDispatcher;
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

inline ThreadPool* Engine::getThreadPool()
{
   return mOpDispatcher->getThreadPool();
}

inline OperationDispatcher* Engine::getOperationDispatcher()
{
   return mOpDispatcher;
}
