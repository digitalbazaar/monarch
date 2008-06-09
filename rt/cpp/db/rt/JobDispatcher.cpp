/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/JobDispatcher.h"

#include <list>
#include <algorithm>

using namespace std;
using namespace db::rt;

JobDispatcher::JobDispatcher()
{
   // create the thread pool with 10 threads by default
   mThreadPool = new ThreadPool(10);
   mCleanupThreadPool = true;
   
   // no dispatcher thread yet
   mDispatcherThread = NULL;
   
   // set thread expire time to 2 minutes (120000 milliseconds) by default
   getThreadPool()->setThreadExpireTime(120000);
}

JobDispatcher::JobDispatcher(ThreadPool* pool, bool cleanupPool)
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

Thread* JobDispatcher::getDispatcherThread()
{
   return mDispatcherThread;
}
   
void JobDispatcher::queueJob(Runnable& job)
{
   lock();
   {
      // add the job to the queue
      mJobQueue.push_back(&job);
   }
   unlock();
   
   // wake up dispatcher
   wakeup();
}

void JobDispatcher::queueJob(RunnableRef& job)
{
   lock();
   {
      // add the job to the queue
      mJobQueue.push_back(&(*job));
      
      // add job to the reference map
      mJobReferenceMap.insert(make_pair(&(*job), job));
   }
   unlock();
   
   // wake up dispatcher
   wakeup();
}

void JobDispatcher::dequeueJob(Runnable& job)
{
   lock();
   {
      // remove the job from the queue
      mJobQueue.remove(&job);
      
      // remove job from the reference map
      mJobReferenceMap.erase(&job);
   }
   unlock();
   
   // wake up dispatcher
   wakeup();
}

void JobDispatcher::dequeueJob(RunnableRef& job)
{
   dequeueJob(*job);
}

void JobDispatcher::dispatchJobs()
{
   lock();
   {
      // try to run all jobs in the queue
      bool run = true;
      for(RunnableList::iterator i = mJobQueue.begin();
          run && i != mJobQueue.end();)
      {
         if(getThreadPool()->tryRunJob(*(*i)))
         {
            // remove entry from map and queue
            mJobReferenceMap.erase(*i);
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

bool JobDispatcher::isQueued(Runnable& job)
{
   bool rval = false;
   
   lock();
   {
      RunnableList::iterator i =
         find(mJobQueue.begin(), mJobQueue.end(), &job);
      rval = (i != mJobQueue.end());
   }
   unlock();
   
   return rval;
}

bool JobDispatcher::isQueued(RunnableRef& job)
{
   return isQueued(*job);
}

void JobDispatcher::startDispatching()
{
   lock();
   {
      if(!isDispatching())
      {
         // create new dispatcher thread
         mDispatcherThread = new Thread(this);
         
         // start dispatcher thread (128k stack)
         mDispatcherThread->start(131072);
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
      // clear queue and map
      mJobQueue.clear();
      mJobReferenceMap.clear();
   }
   unlock();
   
   // wake up dispatcher
   wakeup();
}

void JobDispatcher::interruptAllRunningJobs()
{
   getThreadPool()->interruptAllThreads();
}

void JobDispatcher::terminateAllRunningJobs()
{
   getThreadPool()->terminateAllThreads();
}

ThreadPool* JobDispatcher::getThreadPool()
{
   return mThreadPool;
}

unsigned int JobDispatcher::getQueuedJobCount()
{
   return mJobQueue.size();
}

unsigned int JobDispatcher::getTotalJobCount()
{
   unsigned int rval = 0;
   
   lock();
   {
      rval = getQueuedJobCount() + getThreadPool()->getIdleThreadCount();
   }
   unlock();
   
   return rval;
}
