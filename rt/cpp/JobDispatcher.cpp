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
   
   // no dispatcher thread yet
   mDispatcherThread = NULL;
   
   // set thread expire time to 2 minutes (120000 milliseconds) by default
   getThreadPool()->setJobThreadExpireTime(120000);
}

JobDispatcher::JobDispatcher(JobThreadPool* pool, bool cleanupPool)
{
   // store the thread pool
   mThreadPool = pool;
   mCleanupThreadPool = cleanupPool;
   
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
         mJobQueue.push_back(job);
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

void JobDispatcher::startDispatching()
{
   // synchronize
   lock();
   {
      if(!isDispatching())
      {
         // create new dispatcher thread
         mDispatcherThread = new Thread(this);
         
         // start dispatcher thread
         mDispatcherThread->start();
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
         // interrupt dispatcher thread
         getDispatcherThread()->interrupt();
         
         // join dispatcher thread
         getDispatcherThread()->join();
         
         // clean up dispatcher thread
         delete mDispatcherThread;
         mDispatcherThread = NULL;
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

void JobDispatcher::terminateAllRunningJobs()
{
   // synchronize
   lock();
   {
      getThreadPool()->terminateAllThreads();
   }
   unlock();
}

void JobDispatcher::run()
{
   while(!Thread::interrupted(false))
   {
      // dispatch the next Runnable job
      dispatchNextJob();
      
      // sleep
      Thread::sleep(1);
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
