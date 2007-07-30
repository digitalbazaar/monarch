/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "OperationDispatcher.h"
#include "Engine.h"
#include "OperationExecutor.h"

using namespace std;
using namespace db::modest;
using namespace db::rt;

OperationDispatcher::OperationDispatcher(Engine* e)
{
   mEngine = e;
   mDispatch = false;
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
   OperationExecutor* e = NULL;
   
   // lock state
   mEngine->getState()->lock();
   
   lock();
   {
      // turn off dispatching until an Operation executes or is canceled
      mDispatch = false;
      
      // execute all Operations that can be executed
      for(list<Runnable*>::iterator i = mJobQueue.begin();
          e == NULL && i != mJobQueue.end();)
      {
         e = (OperationExecutor*)(*i);
         switch(e->checkGuard())
         {
            case 0:
               // Operation is executable
               mDispatch = true;
               i = mJobQueue.erase(i);
               
               // run pre-execution state mutation
               e->doPreExecutionStateMutation();
               
               // try to run the operation
               if(getThreadPool()->tryRunJob(e))
               {
                  // Operation executed, no need to run it outside of loop
                  e = NULL;
               }
               break;
            case 1:
               // move to next Operation
               i++;
               e = NULL;
               break;
            case 2:
               // Operation is canceled
               i = mJobQueue.erase(i);
               addExpiredExecutor(e);
               e = NULL;
               break;
         }
      }
   }
   unlock();
   
   // unlock state
   mEngine->getState()->unlock();
   
   if(e != NULL)
   {
      // execute Operation, allow thread blocking
      getThreadPool()->runJob(e);
   }
}

void OperationDispatcher::cleanupExpiredExecutors()
{
   mExpiredExecutorsLock.lock();
   {
      for(list<OperationExecutor*>::iterator i = mExpiredExecutors.begin();
          i != mExpiredExecutors.end();)
      {
         OperationExecutor* e = *i;
         
         // clean up executor
         e->cleanup();
         i = mExpiredExecutors.erase(i);
         delete e;
      }
   }
   mExpiredExecutorsLock.unlock();
}

void OperationDispatcher::queueOperation(OperationExecutor* e)
{
   mDispatch = true;
   JobDispatcher::queueJob(e);
}

void OperationDispatcher::startDispatching()
{
   JobDispatcher::startDispatching();
}

void OperationDispatcher::stopDispatching()
{
   JobDispatcher::stopDispatching();
   
   // clean up any expired executors
   cleanupExpiredExecutors();
}

void OperationDispatcher::run()
{
   Thread* t = Thread::currentThread();
   while(!t->isInterrupted())
   {
      if(!mExpiredExecutors.empty())
      {
         // clean up any expired executors
         cleanupExpiredExecutors();
      }
      
      if(canDispatch())
      {
         dispatchJobs();
      }
      else
      {
         mWaitLock.lock();
         {
            if(!canDispatch())
            {
               mWaitLock.wait();
            }
         }
         mWaitLock.unlock();
      }
   }
}

void OperationDispatcher::clearQueuedOperations()
{
   lock();
   {
      mExpiredExecutorsLock.lock();
      {
         // expire OperationExecutors in the queue
         for(list<Runnable*>::iterator i = mJobQueue.begin();
             i != mJobQueue.end();)
         {
            OperationExecutor* e = (OperationExecutor*)(*i);
            i = mJobQueue.erase(i);
            addExpiredExecutor(e);
         }
         
         // cleanup expired executors
         cleanupExpiredExecutors();
      }
      mExpiredExecutorsLock.unlock();
   }
   unlock();
   
   // wake up dispatcher
   wakeup();
}

void OperationDispatcher::terminateRunningOperations()
{
   JobDispatcher::terminateAllRunningJobs();
   
   // clean up any expired executors
   cleanupExpiredExecutors();
   
   // wake up dispatcher
   wakeup();
}

void OperationDispatcher::addExpiredExecutor(OperationExecutor* e)
{
   mExpiredExecutorsLock.lock();
   {
      mExpiredExecutors.push_back(e);
      mDispatch = true;
   }
   mExpiredExecutorsLock.unlock();
   
   // wake up dispatcher
   wakeup();
}

JobThreadPool* OperationDispatcher::getThreadPool()
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
