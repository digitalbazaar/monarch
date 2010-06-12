/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/modest/Engine.h"

using namespace std;
using namespace monarch::rt;
using namespace monarch::modest;

Engine::Engine() :
   ThreadPool(100),
   JobDispatcher(this, false),
   mDispatch(false)
{
   // set thread expire time to 2 minutes (120000 milliseconds) by default
   setThreadExpireTime(120000);
}

Engine::~Engine()
{
   // ensure engine is stopped
   stop();
}

void Engine::start()
{
   mStartLock.lock();
   {
      startDispatching();
   }
   mStartLock.unlock();
}

void Engine::stop()
{
   mStartLock.lock();
   {
      // stop dispatching
      stopDispatching();

      // terminate all running operations
      terminateRunningOperations();

      // clear all queued operations
      clearQueuedOperations();
   }
   mStartLock.unlock();
}

void Engine::queue(Operation& op)
{
   mLock.lock();
   {
      // ensure to enable dispatching, then add operation to queue and map
      mDispatch = true;
      mJobQueue.push_back(&(*op));
      mOpMap.insert(make_pair(&(*op), op));

      // wake up dispatcher inside lock to ensure dispatch flag doesn't change
      wakeup();
   }
   mLock.unlock();
}

void Engine::clearQueuedOperations()
{
   mLock.lock();
   {
      // remove all job queue entries from the map
      for(list<Runnable*>::iterator i = mJobQueue.begin();
          i != mJobQueue.end(); ++i)
      {
         mOpMap.erase(static_cast<OperationImpl*>(*i));
      }

      // clear queue, wake up
      mJobQueue.clear();
      wakeup();
   }
   mLock.unlock();
}

void Engine::terminateRunningOperations()
{
   JobDispatcher::terminateAllRunningJobs();

   // wake up dispatcher, don't care if dispatch flag changes no need to lock
   wakeup();
}

void Engine::jobCompleted(PooledThread* t)
{
   mLock.lock();
   {
      // Note: this method is executed by a PooledThread, external to an
      // Operation, so that the Operation can be safely garbage-collected
      // here if the map happens to hold the last reference to it

      // get operation reference
      OperationImpl* impl = static_cast<OperationImpl*>(t->getJob());
      OperationMap::iterator i = mOpMap.find(impl);
      Operation& op = i->second;

      // do post-execution state mutation
      StateMutator* sm = op->getStateMutator();
      if(sm != NULL)
      {
         sm->mutatePostExecutionState(op);
      }

      // stop operation, resume dispatching
      op->stop();
      mDispatch = true;
      wakeup();

      // remove operation reference from map
      mOpMap.erase(i);
   }
   mLock.unlock();

   // call parent method to release thread back into pool
   ThreadPool::jobCompleted(t);
}

Operation Engine::getCurrentOperation()
{
   Operation rval(NULL);

   // get the current thread's OperationImpl
   Thread* thread = Thread::currentThread();
   OperationImpl* impl = static_cast<OperationImpl*>(thread->getUserData());
   if(impl != NULL)
   {
      mLock.lock();
      OperationMap::iterator i = mOpMap.find(impl);
      if(i != mOpMap.end())
      {
         rval = i->second;
      }
      mLock.unlock();
   }

   return rval;
}

inline ThreadPool* Engine::getThreadPool()
{
   return this;
}

unsigned int Engine::getQueuedOperationCount()
{
   return JobDispatcher::getQueuedJobCount();
}

unsigned int Engine::getTotalOperationCount()
{
   return JobDispatcher::getTotalJobCount();
}

bool Engine::canDispatch()
{
   return mDispatch;
}

void Engine::dispatchJobs()
{
   OperationImpl* impl = NULL;

   mLock.lock();
   {
      // turn off dispatching (will be turned back on when state is modified
      // or an operation starts/completes)
      mDispatch = false;

      // FIXME: ease lock contention when queuing vs. dispatching jobs

      // execute all Operations that can be executed
      Operation* op;
      StateMutator* sm;
      OperationGuard* og;
      list<Runnable*>::iterator end = mJobQueue.end();
      for(list<Runnable*>::iterator i = mJobQueue.begin();
          impl == NULL && i != end;)
      {
         impl = static_cast<OperationImpl*>(*i);

         // get guard and mutator
         og = impl->getGuard();
         sm = impl->getStateMutator();

         // get reference to operation if op has a guard or mutator
         op = (og != NULL || sm != NULL) ? &mOpMap[impl] : NULL;

         // check the operation's guard restrictions
         if(og == NULL || og->canExecuteOperation(*op))
         {
            // operation can execute, unqueue, advance iterator, dispatch on
            i = mJobQueue.erase(i);
            mDispatch = true;

            // do pre-execution state mutation
            if(sm != NULL)
            {
               sm->mutatePreExecutionState(*op);
            }

            // try to run the operation without blocking
            if(tryRunJob(*impl))
            {
               // operation executed, no need to run it outside of loop
               impl = NULL;
            }
         }
         // operation can wait
         else if(!impl->isInterrupted() && !og->mustCancelOperation(*op))
         {
            // move to next op in the queue
            impl = NULL;
            ++i;
         }
         // operation must be canceled
         else
         {
            // stop, unmap, and unqueue op
            impl->stop();
            mOpMap.erase(impl);
            i = mJobQueue.erase(i);
            impl = NULL;
         }
      }
   }
   mLock.unlock();

   if(impl != NULL)
   {
      // execute Operation, allow thread blocking
      runJob(*impl);
   }
}
