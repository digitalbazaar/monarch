/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/rt/JobDispatcher.h"

#include <list>
#include <algorithm>

using namespace std;
using namespace monarch::rt;

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

inline void JobDispatcher::wakeup()
{
   // wake up dispatcher
   mLock.notifyAll();
}

inline bool JobDispatcher::canDispatch()
{
   return !mJobQueue.empty();
}

void JobDispatcher::queueJob(Runnable& job)
{
   mLock.lock();
   {
      // add the job to the queue and wakeup
      mJobQueue.push_back(&job);
      wakeup();
   }
   mLock.unlock();
}

void JobDispatcher::queueJob(RunnableRef& job)
{
   mLock.lock();
   {
      // add the job to the queue and reference map, wakeup
      mJobQueue.push_back(&(*job));
      mJobReferenceMap.insert(make_pair(&(*job), job));
      wakeup();
   }
   mLock.unlock();
}

void JobDispatcher::dequeueJob(Runnable& job)
{
   mLock.lock();
   {
      // remove the job from the queue, reference map, wakeup
      mJobQueue.remove(&job);
      mJobReferenceMap.erase(&job);
      wakeup();
   }
   mLock.unlock();
}

inline void JobDispatcher::dequeueJob(RunnableRef& job)
{
   dequeueJob(*job);
}

void JobDispatcher::dispatchJobs()
{
   mLock.lock();
   {
      // try to run all jobs in the queue
      bool run = true;
      for(RunnableList::iterator i = mJobQueue.begin();
          run && i != mJobQueue.end();)
      {
         // use RunnableRef if one exists
         ReferenceMap::iterator mi = mJobReferenceMap.find(*i);
         if(mi != mJobReferenceMap.end())
         {
            if(getThreadPool()->tryRunJob(mi->second))
            {
               // remove entry from map and queue
               mJobReferenceMap.erase(mi);
               i = mJobQueue.erase(i);
            }
            else
            {
               run = false;
            }
         }
         else if(getThreadPool()->tryRunJob(*(*i)))
         {
            // remove entry from queue
            i = mJobQueue.erase(i);
         }
         else
         {
            run = false;
         }
      }
   }
   mLock.unlock();
}

bool JobDispatcher::isQueued(Runnable& job)
{
   bool rval = false;

   mLock.lock();
   {
      RunnableList::iterator i = find(mJobQueue.begin(), mJobQueue.end(), &job);
      rval = (i != mJobQueue.end());
   }
   mLock.unlock();

   return rval;
}

inline bool JobDispatcher::isQueued(RunnableRef& job)
{
   return isQueued(*job);
}

void JobDispatcher::startDispatching()
{
   mLock.lock();
   {
      if(!isDispatching())
      {
         // create new dispatcher thread
         mDispatcherThread = new Thread(this);

         // start dispatcher thread (128k stack)
         mDispatcherThread->start(131072);
      }
   }
   mLock.unlock();
}

void JobDispatcher::stopDispatching()
{
   Thread* t = NULL;

   mLock.lock();
   {
      if(isDispatching())
      {
         // interrupt dispatcher thread
         t = mDispatcherThread;
         t->interrupt();
         mDispatcherThread = NULL;
      }
   }
   mLock.unlock();

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
         mLock.lock();
         if(!canDispatch())
         {
            mLock.wait();
         }
         mLock.unlock();
      }
   }
}

bool JobDispatcher::isDispatching()
{
   bool rval = false;

   mLock.lock();
   {
      rval = (mDispatcherThread != NULL);
   }
   mLock.unlock();

   return rval;
}

void JobDispatcher::clearQueuedJobs()
{
   mLock.lock();
   {
      // clear queue and map, wakeup
      mJobQueue.clear();
      mJobReferenceMap.clear();
      wakeup();
   }
   mLock.unlock();
}

inline void JobDispatcher::interruptAllRunningJobs()
{
   getThreadPool()->interruptAllThreads();
}

inline void JobDispatcher::terminateAllRunningJobs()
{
   getThreadPool()->terminateAllThreads();
}

inline ThreadPool* JobDispatcher::getThreadPool()
{
   return mThreadPool;
}

inline unsigned int JobDispatcher::getQueuedJobCount()
{
   return mJobQueue.size();
}

unsigned int JobDispatcher::getTotalJobCount()
{
   unsigned int rval = 0;

   mLock.lock();
   {
      rval = getQueuedJobCount() + getThreadPool()->getRunningThreadCount();
   }
   mLock.unlock();

   return rval;
}
