/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/JobThreadPool.h"

using namespace std;
using namespace db::rt;

JobThreadPool::JobThreadPool(unsigned int poolSize) :
   mThreadSemaphore(poolSize, true)
{
   // default JobThread expire time to 0 (no expiration)
   mJobThreadExpireTime = 0;
}

JobThreadPool::~JobThreadPool()
{
   // terminate all threads
   terminateAllThreads();
}

JobThread* JobThreadPool::getIdleThread()
{
   JobThread* rval = NULL;
   
   mListLock.lock();
   {
      while(rval == NULL && !mIdleThreads.empty())
      {
         rval = mIdleThreads.front();
         mIdleThreads.pop_front();
         
         if(rval->isExpired())
         {
            // collect thread for clean up
            mThreads.remove(rval);
            mExpiredThreads.push_front(rval);
            rval = NULL;
         }
      }
      
      if(rval == NULL)
      {
         // clean up expired threads
         cleanupExpiredThreads();
         
         // create new job thread and add to thread list
         rval = new JobThread(getJobThreadExpireTime());
         mThreads.push_back(rval);
      }
   }
   mListLock.unlock();
   
   // clean up extra threads if applicable
   if(!mIdleThreads.empty() && mThreadSemaphore.getMaxPermitCount() > 0)
   {
      // get the number of extra threads
      int extraThreads =
         mThreads.size() - mThreadSemaphore.getMaxPermitCount();
      
      // remove extra idle threads
      if(extraThreads > 0)
      {
         removeIdleThreads((unsigned int)extraThreads);
      }
   }
   
   return rval;
}

void JobThreadPool::removeIdleThreads(unsigned int count)
{
   if(!mIdleThreads.empty())
   {
      mListLock.lock();
      {
         for(list<JobThread*>::iterator i = mIdleThreads.begin();
             count > 0 && i != mIdleThreads.end(); count--)
         {
            // interrupt and erase threads
            JobThread* t = *i;
            t->interrupt();
            i = mIdleThreads.erase(i);
            mThreads.remove(t);
            mExpiredThreads.push_back(t);
         }
         
         // clean up expired threads
         cleanupExpiredThreads();
      }
      mListLock.unlock();
   }
}

void JobThreadPool::cleanupExpiredThreads()
{
   if(!mExpiredThreads.empty())
   {
      // assume list lock is engaged
      // join and clean up threads
      for(list<JobThread*>::iterator i = mExpiredThreads.begin();
          i != mExpiredThreads.end(); i++)
      {
         (*i)->join();
         delete (*i);
      }
      
      // clear expired list
      mExpiredThreads.clear();
   }
}

void JobThreadPool::runJobOnIdleThread(Runnable* job)
{
   lock();
   {
      // get an idle thread
      JobThread* t = getIdleThread();
      
      // set job
      t->setJob(job, this);
      
      // if the thread hasn't started yet, start it
      if(!t->hasStarted())
      {
         t->start();
      }
   }
   unlock();
}

bool JobThreadPool::tryRunJob(Runnable* job)
{
   bool rval = false;
   
   if(job != NULL)
   {
      // only try to acquire a permit if infinite threads is not enabled
      rval = true;
      if(mThreadSemaphore.getMaxPermitCount() != 0)
      {
         rval = mThreadSemaphore.tryAcquire();
      }
      
      if(rval)
      {
         // run the job on an idle thread
         runJobOnIdleThread(job);
      }
   }
   
   return rval;
}

void JobThreadPool::runJob(Runnable* job)
{
   if(job != NULL)
   {
      // only acquire a permit if infinite threads is not enabled
      bool permitAcquired = true;
      if(mThreadSemaphore.getMaxPermitCount() != 0)
      {
         permitAcquired = (mThreadSemaphore.acquire() == NULL);
      }
      
      if(permitAcquired)
      {
         // run the job on an idle thread
         runJobOnIdleThread(job);
      }
   }
}

void JobThreadPool::jobCompleted(JobThread* t)
{
   // clear the thread's job
   t->setJob(NULL, NULL);
   
   mListLock.lock();
   {
      // add the job to the front of the idle list, so it is more
      // likely to get assigned immediately
      mIdleThreads.push_front(t);
   }
   mListLock.unlock();
   
   // release thread permit
   mThreadSemaphore.release();
}

void JobThreadPool::interruptAllThreads()
{
   lock();
   {
      mListLock.lock();
      {
         // interrupt all threads
         for(list<JobThread*>::iterator i = mThreads.begin();
             i != mThreads.end(); i++)
         {
            (*i)->interrupt();
         }
      }
      mListLock.unlock();
   }
   unlock();
}

void JobThreadPool::terminateAllThreads()
{
   lock();
   {
      // interrupt all the threads
      interruptAllThreads();
      
      // join and remove all threads
      for(list<JobThread*>::iterator i = mThreads.begin();
          i != mThreads.end();)
      {
         JobThread* t = *i;
         i = mThreads.erase(i);
         t->join();
         delete t;
      }
      
      // clear threads
      mThreads.clear();
      mIdleThreads.clear();
   }
   unlock();
}

void JobThreadPool::setPoolSize(unsigned int size)
{
   lock();
   {
      // Note: threads are created lazily so if the thread pool size
      // here is greater than the number of threads currently in
      // the pool, those threads will be created as they are needed
      // hence, we do not need to adjust for increases in the pool size
      // only for decreases
      
      // remove threads as necessary
      if(mThreads.size() > size && size != 0)
      {
         removeIdleThreads(mThreads.size() - size);
      }
      
      // set semaphore permits
      mThreadSemaphore.setMaxPermitCount(size);
   }
   unlock();
}

unsigned int JobThreadPool::getPoolSize()
{
   return mThreadSemaphore.getMaxPermitCount();
}

void JobThreadPool::setJobThreadExpireTime(unsigned long long expireTime)
{
   lock();
   {
      mJobThreadExpireTime = expireTime;
      
      mListLock.lock();
      {
         // update all existing job threads
         for(list<JobThread*>::iterator i = mThreads.begin();
             i != mThreads.end(); i++)
         {
            (*i)->setExpireTime(expireTime);
         }
      }
      mListLock.unlock();
   }
   unlock();
}

unsigned long long JobThreadPool::getJobThreadExpireTime()
{
   return mJobThreadExpireTime;
}

unsigned int JobThreadPool::getJobThreadCount()
{
   return mThreads.size();
}

unsigned int JobThreadPool::getRunningJobThreadCount()
{
   unsigned int rval = 0;
   
   mListLock.lock();
   {
      // subtract idle threads from total threads
      rval = mThreads.size() - mIdleThreads.size();
   }
   mListLock.unlock();
   
   return rval;
}

unsigned int JobThreadPool::getIdleJobThreadCount()
{
   return mIdleThreads.size();
}
