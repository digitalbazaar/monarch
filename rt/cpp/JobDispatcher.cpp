/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "JobDispatcher.h"

using namespace std;
using namespace db::rt;

JobDispatcher::JobDispatcher()
{
   // create the job thread pool with an infinite number of
   // threads by default
   mThreadPool = new JobThreadPool(0);
   mCleanupThreadPool = true;
   
   // set thread expire time to 5 minutes (300000 milliseconds) by default
   getThreadPool()->setJobThreadExpireTime(300000);
}

JobDispatcher::JobDispatcher(JobThreadPool* pool, bool cleanupPool)
{
   // store the thread pool
   mThreadPool = pool;
   
   // no dispatcher thread yet
   mDispatcherThread = NULL;
}

JobDispatcher::~JobDispatcher()
{
   // clean up dispatcher thread as appropriate
   if(mDispatcherThread != NULL)
   {
      delete mDispatcherThread;
   }
   
   // clean up thread pool as appropriate
   if(mCleanupThreadPool && mThreadPool != NULL)
   {
      delete mThreadPool;
   }
}

bool JobDispatcher::pushJob(Runnable* job)
{
   bool rval = false;
   
   if(job != NULL)
   {
      // synchronize
      lock();
      {
         // add the job to the queue
         mJobQueue.push_front(job);
         rval = true;
      }
      unlock();
   }
   
   return rval;
}

Runnable* JobDispatcher::popJob() 
{
   Runnable* rval = NULL;
   
   // synchronize
   lock();
   {
      if(!mJobQueue.empty())
      {
         // remove the top Runnable job
         rval = mJobQueue.front();
         mJobQueue.pop_front();
      }
   }
   unlock();
   
   return rval;
}

list<Runnable*>::iterator JobDispatcher::getJobIterator()
{
   return mJobQueue.begin();
}

Thread* JobDispatcher::getDispatcherThread()
{
   return mDispatcherThread;
}
   
void JobDispatcher::queueJob(Runnable* job)
{
   pushJob(job);
}

void JobDispatcher::dequeueJob(Runnable* job)
{
   // synchronize
   lock();
   {
      mJobQueue.remove(job);
   }
   unlock();
}

void JobDispatcher::dispatchNextJob()
{
   // pop next Runnable job off of the queue
   Runnable* job = popJob();
   if(job != NULL)
   {
      // run the job
      getThreadPool()->runJob(job);      
   }
}

bool JobDispatcher::isQueued(Runnable* job)
{
   bool rval = false;
   
   // synchronize
   lock();
   {
      list<Runnable*>::iterator i =
         find(mJobQueue.begin(), mJobQueue.end(), job);
      rval = (i != mJobQueue.end());
   }
   unlock();
   
   return rval;
}

// FIXME: remove iostream
#include <iostream>
void JobDispatcher::startDispatching()
{
   // synchronize
   lock();
   {
      if(!isDispatching())
      {
         cout << "JobDispatcher starting dispatching..." << endl;
         
         // create new dispatcher thread
         mDispatcherThread = new Thread(this);
         
         // start dispatcher thread
         mDispatcherThread->start();
         
         cout << "JobDispatcher started dispatching." << endl;
      }
      else
      {
         cout << "JobDispatcher is already dispatching." << endl;
      }
   }
   unlock();
}

void JobDispatcher::stopDispatching()
{
   // synchronize
   lock();
   {
      if(isDispatching())
      {
         cout << "JobDispatcher stopping dispatching..." << endl;
         
         // interrupt dispatcher thread
         getDispatcherThread()->interrupt();
         
         // clean up dispatcher thread
         mDispatcherThread = NULL;
         
         cout << "JobDispatcher stopped dispatching." << endl;
      }
      else
      {
         cout << "JobDispatcher is already not dispatching." << endl;
      }
   }
   unlock();
}

bool JobDispatcher::isDispatching()
{
   bool rval = false;
   
   // synchronize
   lock();
   {
      rval = (getDispatcherThread() != NULL);
   }
   unlock();
   
   return rval;
}

void JobDispatcher::clearQueuedJobs()
{
   // synchronize
   lock();
   {
      mJobQueue.clear();
   }
   unlock();
}

void JobDispatcher::interruptAllRunningJobs()
{
   // synchronize
   lock();
   {
      getThreadPool()->interruptAllThreads();
   }
   unlock();
}

void JobDispatcher::terminateAllRunningJobs(unsigned long long joinTime)
{
   // synchronize
   lock();
   {
      getThreadPool()->terminateAllThreads(joinTime);
   }
   unlock();
}

void JobDispatcher::run()
{
   try
   {
      while(!Thread::currentThread()->isInterrupted())
      {
         // dispatch the next Runnable job
         dispatchNextJob();
         
         // sleep
         Thread::sleep(1);
      }
   }
   catch(InterruptedException e)
   {
      // toggle interrupted flag to true
      Thread::currentThread()->interrupt();
   }
}

JobThreadPool* JobDispatcher::getThreadPool()
{
   return mThreadPool;
}

unsigned int JobDispatcher::getQueuedJobCount()
{
   unsigned int rval = 0;
   
   // synchronize
   lock();
   {
      rval = mJobQueue.size();
   }
   unlock();
   
   return rval;
}

unsigned int JobDispatcher::getTotalJobCount()
{
   unsigned int rval = 0;
   
   // synchronize
   lock();
   {
      rval = getQueuedJobCount() + getThreadPool()->getIdleJobThreadCount();
   }
   unlock();
   
   return rval;
}
