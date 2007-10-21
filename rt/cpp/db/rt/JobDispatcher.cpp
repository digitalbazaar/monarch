/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/JobDispatcher.h"

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

Runnable* JobDispatcher::popJob() 
{
   Runnable* rval = NULL;
   
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

void JobDispatcher::wakeup()
{
   mWaitLock.lock();
   {
      // wake up dispatcher
      mWaitLock.notifyAll();
   }
   mWaitLock.unlock();
}

bool JobDispatcher::canDispatch()
{
   return !mJobQueue.empty();
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
   if(job != NULL)
   {
      lock();
      {
         // add the job to the queue
         mJobQueue.push_back(job);
      }
      unlock();
      
      // wake up dispatcher
      wakeup();
   }
}

void JobDispatcher::dequeueJob(Runnable* job)
{
   lock();
   {
      // remove the job from the queue
      mJobQueue.remove(job);
   }
   unlock();
   
   // wake up dispatcher
   wakeup();
}

void JobDispatcher::dispatchJobs()
{
   lock();
   {
      // try to run all jobs in the queue
      bool run = true;
      for(list<Runnable*>::iterator i = mJobQueue.begin();
          run && i != mJobQueue.end();)
      {
         if(getThreadPool()->tryRunJob(*i))
         {
            i = mJobQueue.erase(i);
         }
         else
         {
            run = false;
         }
      }
   }
   unlock();
}

bool JobDispatcher::isQueued(Runnable* job)
{
   bool rval = false;
   
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
   Thread* t = NULL;
   
   lock();
   {
      if(isDispatching())
      {
         // interrupt dispatcher thread
         t = getDispatcherThread();
         t->interrupt();
         mDispatcherThread = NULL;
      }
   }
   unlock();
   
   if(t != NULL)
   {
      // join and clean up old dispatcher thread
      t->join();
      delete t;
   }
}

void JobDispatcher::run()
{
   Thread* t = Thread::currentThread();
   while(!t->isInterrupted())
   {
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

bool JobDispatcher::isDispatching()
{
   bool rval = false;
   
   lock();
   {
      rval = (getDispatcherThread() != NULL);
   }
   unlock();
   
   return rval;
}

void JobDispatcher::clearQueuedJobs()
{
   lock();
   {
      mJobQueue.clear();
   }
   unlock();
}

void JobDispatcher::interruptAllRunningJobs()
{
   lock();
   {
      getThreadPool()->interruptAllThreads();
   }
   unlock();
}

void JobDispatcher::terminateAllRunningJobs()
{
   lock();
   {
      getThreadPool()->terminateAllThreads();
   }
   unlock();
}

JobThreadPool* JobDispatcher::getThreadPool()
{
   return mThreadPool;
}

unsigned int JobDispatcher::getQueuedJobCount()
{
   unsigned int rval = 0;
   
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
   
   lock();
   {
      rval = getQueuedJobCount() + getThreadPool()->getIdleJobThreadCount();
   }
   unlock();
   
   return rval;
}
