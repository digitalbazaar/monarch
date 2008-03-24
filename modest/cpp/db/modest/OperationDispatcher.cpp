/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/OperationDispatcher.h"
#include "db/modest/Engine.h"

using namespace std;
using namespace db::modest;
using namespace db::rt;

OperationDispatcher::OperationDispatcher(Engine* e) :
   ThreadPool(200),
   JobDispatcher(this, false)
{
   mEngine = e;
   mDispatch = false;
   
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
   ImmutableState* state = mEngine->getState();
   
   lock();
   {
      // turn off dispatching until an Operation executes or is canceled
      mDispatch = false;
      
      // execute all Operations that can be executed
      int guardCheck;
      for(list<Runnable*>::iterator i = mJobQueue.begin();
          impl == NULL && i != mJobQueue.end();)
      {
         impl = (OperationImpl*)*i;
         
         // lock engine state
         state->lock();
         {
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
                        (State*)state, mOpMap[impl]);
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
         // unlock engine state
         state->unlock();
      }
   }
   unlock();
   
   if(impl != NULL)
   {
      // execute Operation, allow thread blocking
      getThreadPool()->runJob(*impl);
   }
}

void OperationDispatcher::queueOperation(Operation& op)
{
   lock();
   {
      // ensure to enable dispatching, then add operation to queue and map
      mDispatch = true;
      mJobQueue.push_back(&(*op));
      mOpMap.insert(make_pair(&(*op), op));
   }
   unlock();
   
   // wake up dispatcher
   wakeup();
}

void OperationDispatcher::clearQueuedOperations()
{
   lock();
   {
      // remove all job queue entries from the map
      for(list<Runnable*>::iterator i = mJobQueue.begin();
          i != mJobQueue.end(); i++)
      {
         mOpMap.erase((OperationImpl*)*i);
      }
      
      // clear queue
      mJobQueue.clear();
   }
   unlock();
   
   // wake up dispatcher
   wakeup();
}

void OperationDispatcher::terminateRunningOperations()
{
   JobDispatcher::terminateAllRunningJobs();
   
   // wake up dispatcher
   wakeup();
}

void OperationDispatcher::jobCompleted(PooledThread* t)
{
   lock();
   {
      // Note: this method is executed by a PooledThread, external to an
      // Operation, so that the Operation can be safely garbage-collected
      // here if the map happens to hold the last reference to it 
      
      // get operation reference
      OperationImpl* impl = (OperationImpl*)t->getJob();
      OperationMap::iterator i = mOpMap.find(impl);
      Operation& op = i->second;
      
      // do post-execution state mutation
      if(op->getStateMutator() != NULL)
      {
         mEngine->getState()->lock();
         {
            op->getStateMutator()->mutatePostExecutionState(
               (State*)mEngine->getState(), op);
         }
         mEngine->getState()->unlock();
      }
      
      // stop operation
      op->stop();
      
      // wake up dispatcher
      mDispatch = true;
      wakeup();
      
      // remove operation reference from map
      mOpMap.erase(i);
   }
   unlock();
   
   // call parent method to release thread back into pool
   ThreadPool::jobCompleted(t);
}

Operation OperationDispatcher::getCurrentOperation()
{
   // get the current thread's OperationImpl
   Thread* thread = Thread::currentThread();
   OperationImpl* impl = (OperationImpl*)thread->getUserData();
   OperationMap::iterator i = mOpMap.find(impl);
   return i->second;
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
