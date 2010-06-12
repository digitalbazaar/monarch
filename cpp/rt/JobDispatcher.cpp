/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/rt/JobDispatcher.h"

#include <list>
#include <algorithm>

using namespace std;
using namespace monarch::rt;

JobDispatcher::JobDispatcher() :
   mThreadPool(new ThreadPool(10)),
   mCleanupThreadPool(true),
   mDispatcherThread(NULL)
{
   // set thread expire time to 2 minutes (120000 milliseconds) by default
   mThreadPool->setThreadExpireTime(120000);
}

JobDispatcher::JobDispatcher(ThreadPool* pool, bool cleanupPool) :
   mThreadPool(pool),
   mCleanupThreadPool(cleanupPool),
   mDispatcherThread(NULL)
{
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
      Job j;
      j.type = Job::TypeRunnable;
      j.runnable = &job;
      mJobQueue.push_back(j);
      wakeup();
   }
   mLock.unlock();
}

void JobDispatcher::queueJob(RunnableRef& job)
{
   mLock.lock();
   {
      // add the job to the queue and wakeup
      Job j;
      j.type = Job::TypeRunnableRef;
      j.runnableRef = new RunnableRef(job);
      mJobQueue.push_back(j);
      wakeup();
   }
   mLock.unlock();
}

void JobDispatcher::dequeueJob(Runnable& job)
{
   mLock.lock();
   {
      // find and remove the job from the queue
      bool found = false;
      JobList::iterator end = mJobQueue.end();
      for(JobList::iterator i = mJobQueue.begin(); !found && i != end; i++)
      {
         if(i->type == Job::TypeRunnable && i->runnable == &job)
         {
            // remove from queue
            found = true;
            mJobQueue.erase(i);
         }
         else if(i->type == Job::TypeRunnableRef &&
            &(*(*i->runnableRef)) == &job)
         {
            // delete reference, remove from queue
            found = true;
            delete i->runnableRef;
            mJobQueue.erase(i);
         }
      }
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
      ThreadPool* tp = getThreadPool();
      JobList::iterator end = mJobQueue.end();
      for(JobList::iterator i = mJobQueue.begin(); run && i != end;)
      {
         if(i->type == Job::TypeRunnable &&
            tp->tryRunJob(*i->runnable))
         {
            // remove from queue
            mJobQueue.erase(i);
         }
         else if(i->type == Job::TypeRunnableRef &&
                 tp->tryRunJob(*i->runnableRef))
         {
            // delete reference, remove from queue
            delete i->runnableRef;
            mJobQueue.erase(i);
         }
         else
         {
            // failed to run job
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
      // find the job in the queue
      JobList::iterator end = mJobQueue.end();
      for(JobList::iterator i = mJobQueue.begin(); !rval && i != end; i++)
      {
         if((i->type == Job::TypeRunnable && i->runnable == &job) ||
            (i->type == Job::TypeRunnableRef &&
               &(*(*i->runnableRef)) == &job))
         {
            rval = true;
         }
      }
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
      // clear queue
      JobList::iterator end = mJobQueue.end();
      for(JobList::iterator i = mJobQueue.begin(); i != end; i++)
      {
         if(i->type == Job::TypeRunnableRef)
         {
            delete i->runnableRef;
         }
      }
      mJobQueue.clear();

      // wake up
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
