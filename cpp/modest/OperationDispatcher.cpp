/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/modest/OperationDispatcher.h"

#include "monarch/modest/Engine.h"

using namespace std;
using namespace monarch::modest;
using namespace monarch::rt;

OperationDispatcher::OperationDispatcher(Engine* e) :
   ThreadPool(100),
   JobDispatcher(this, false),
   mEngine(e),
   mDispatch(false)
{
   // set thread expire time to 2 minutes (120000 milliseconds) by default
   getThreadPool()->setThreadExpireTime(120000);
}

OperationDispatcher::~OperationDispatcher()
{
   // stop dispatching
   stopDispatching();

   // terminate all running operations
   terminateRunningOperations();

   // clear all queued operations
   clearQueuedOperations();
}

bool OperationDispatcher::canDispatch()
{
   return mDispatch;
}

void OperationDispatcher::dispatchJobs()
{
   OperationImpl* impl = NULL;

   // get engine state
   State* state = static_cast<State*>(mEngine->getState());

   mLock.lock();
   {
      // turn off dispatching until an Operation executes
      mDispatch = false;

      // FIXME: ease lock contention when queuing vs. dispatching jobs

      // execute all Operations that can be executed
      int guardCheck;
      for(list<Runnable*>::iterator i = mJobQueue.begin();
          impl == NULL && i != mJobQueue.end();)
      {
         impl = static_cast<OperationImpl*>(*i);

         // check the Operation's guard restrictions
         guardCheck = 0;
         if(impl->getGuard() != NULL)
         {
            Operation& op = mOpMap[impl];
            if(!impl->getGuard()->canExecuteOperation(state, op))
            {
               if(!impl->isInterrupted() &&
                  !impl->getGuard()->mustCancelOperation(state, op))
               {
                  // operation can wait
                  guardCheck = 1;
               }
               else
               {
                  // operation must be canceled
                  guardCheck = 2;
               }
            }
         }

         switch(guardCheck)
         {
            case 0:
               // Operation is executable, enable dispatching and unqueue
               mDispatch = true;
               i = mJobQueue.erase(i);

               // do pre-execution state mutation
               if(impl->getStateMutator() != NULL)
               {
                  impl->getStateMutator()->mutatePreExecutionState(
                     state, mOpMap[impl]);
               }

               // try to run the operation
               if(getThreadPool()->tryRunJob(*impl))
               {
                  // Operation executed, no need to run it outside of loop
                  impl = NULL;
               }
               break;
            case 1:
               // move to next Operation
               impl = NULL;
               i++;
               break;
            case 2:
               // Operation is canceled, stop, unmap and unqueue
               impl->stop();
               mOpMap.erase(impl);
               i = mJobQueue.erase(i);
               impl = NULL;
               break;
         }
      }
   }
   mLock.unlock();

   if(impl != NULL)
   {
      // execute Operation, allow thread blocking
      getThreadPool()->runJob(*impl);
   }
}

void OperationDispatcher::queueOperation(Operation& op)
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

void OperationDispatcher::clearQueuedOperations()
{
   mLock.lock();
   {
      // remove all job queue entries from the map
      for(list<Runnable*>::iterator i = mJobQueue.begin();
          i != mJobQueue.end(); i++)
      {
         mOpMap.erase(static_cast<OperationImpl*>(*i));
      }

      // clear queue, wake up
      mJobQueue.clear();
      wakeup();
   }
   mLock.unlock();
}

void OperationDispatcher::terminateRunningOperations()
{
   JobDispatcher::terminateAllRunningJobs();

   // wake up dispatcher, don't care if dispatch flag changes no need to lock
   wakeup();
}

void OperationDispatcher::jobCompleted(PooledThread* t)
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
      if(op->getStateMutator() != NULL)
      {
         State* state = static_cast<State*>(mEngine->getState());
         op->getStateMutator()->mutatePostExecutionState(state, op);
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

Operation OperationDispatcher::getCurrentOperation()
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

ThreadPool* OperationDispatcher::getThreadPool()
{
   return JobDispatcher::getThreadPool();
}

unsigned int OperationDispatcher::getQueuedOperationCount()
{
   return JobDispatcher::getQueuedJobCount();
}

unsigned int OperationDispatcher::getTotalOperationCount()
{
   return JobDispatcher::getTotalJobCount();
}
