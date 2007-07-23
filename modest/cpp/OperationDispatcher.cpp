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
   mDispatch = true;
}
#include <iostream>
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
   // clean up any expired executors
   cleanupExpiredExecutors();
   
   return mDispatch;
}

void OperationDispatcher::dispatchNextJob()
{
   OperationExecutor* e = NULL;
   
   // lock state, executor will unlock it
   mEngine->getState()->lock();
   
   lock();
   {
      // look up the queue until an Operation is found that can be executed
      for(list<Runnable*>::iterator i = mJobQueue.begin();
          e == NULL && i != mJobQueue.end();)
      {
         e = (OperationExecutor*)(*i);
         switch(e->checkGuard())
         {
            case 0:
               // Operation is executable
               i = mJobQueue.erase(i);
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
   
   if(e != NULL)
   {
      // execute Operation
      e->execute();
      mDispatch = true;
   }
   else
   {
      // no executor, so unlock state
      mEngine->getState()->unlock();
      mDispatch = false;
   }
}

void OperationDispatcher::cleanupExpiredExecutors()
{
   lock();
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
   unlock();
}

void OperationDispatcher::queueOperation(OperationExecutor* e)
{
   JobDispatcher::queueJob(e);
   mDispatch = true;
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

void OperationDispatcher::clearQueuedOperations()
{
   lock();
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
   unlock();
}

void OperationDispatcher::terminateRunningOperations()
{
   cout << "terminating all running OPS" << endl;
   JobDispatcher::terminateAllRunningJobs();
   cout << "all running OPS terminated." << endl;
   
   cout << "cleaning all expired EEs" << endl;
   // clean up any expired executors
   cleanupExpiredExecutors();
   cout << "all expired EEs cleaned,EE count=" << mExpiredExecutors.size() << endl;
}

void OperationDispatcher::addExpiredExecutor(OperationExecutor* e)
{
   lock();
   {
      mExpiredExecutors.push_back(e);
      mDispatch = true;
   }
   unlock();
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
