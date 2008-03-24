/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/ThreadPool.h"

using namespace std;
using namespace db::rt;

ThreadPool::ThreadPool(unsigned int poolSize, size_t stackSize) :
   mThreadSemaphore(poolSize, true)
{
   // set stack size
   mThreadStackSize = stackSize;
   
   // default thread expire time to 0 (no expiration)
   mThreadExpireTime = 0;
}

ThreadPool::~ThreadPool()
{
   // terminate all threads
   terminateAllThreads();
}

PooledThread* ThreadPool::getIdleThread()
{
   PooledThread* rval = NULL;
   
   mListLock.lock();
   {
      while(rval == NULL && !mIdleThreads.empty())
      {
         rval = mIdleThreads.front();
         mIdleThreads.pop_front();
         
         // lock thread until it is assigned a job or marked expired
         // 
         // Note: This must be done because the thread could come out of
         // its idle state while we are checking it. If, when we check it,
         // the thread is not expired, then we might accidentally assign it
         // a job, and in the thread's code it will be interrupted just after
         // we check it, resulting in the assigned job never being handled.
         rval->lock();
         if(rval->isExpired() || rval->isInterrupted())
         {
            // unlock thread, collect thread for clean up
            rval->unlock();
            mThreads.remove(rval);
            mExpiredThreads.push_front(rval);
            rval = NULL;
         }
      }
      
      if(rval == NULL)
      {
         // clean up expired threads
         cleanupExpiredThreads();
         
         // create new thread and add it to the thread list
         rval = new PooledThread(getThreadExpireTime());
         mThreads.push_back(rval);
         
         // lock thread as if it were an idle thread so the later
         // unlock code is consistent
         rval->lock();
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

void ThreadPool::removeIdleThreads(unsigned int count)
{
   if(!mIdleThreads.empty())
   {
      mListLock.lock();
      {
         for(ThreadList::iterator i = mIdleThreads.begin();
             count > 0 && i != mIdleThreads.end(); count--)
         {
            // interrupt and erase threads
            PooledThread* t = *i;
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

void ThreadPool::cleanupExpiredThreads()
{
   if(!mExpiredThreads.empty())
   {
      // assume list lock is engaged
      // join and clean up threads
      for(ThreadList::iterator i = mExpiredThreads.begin();
          i != mExpiredThreads.end(); i++)
      {
         (*i)->join();
         delete (*i);
      }
      
      // clear expired list
      mExpiredThreads.clear();
   }
}

void ThreadPool::runJobOnIdleThread(Runnable& job)
{
   lock();
   {
      // get an idle thread
      PooledThread* t = getIdleThread();
      
      // set job
      t->setJob(&job, this);
      
      // unlock thread now that a job is assigned, so if it was about
      // to become unidled, it will pick up its new assignment
      t->unlock();
      
      // if the thread hasn't started yet, start it
      while(!t->hasStarted())
      {
         if(!t->start(mThreadStackSize))
         {
            // FIXME: thread might not start if maximum threads
            // has been exceeded -- we need a better solution than this:
            
            // yield, try to start again later
            Thread::yield();
         }
      }
   }
   unlock();
}

void ThreadPool::runJobOnIdleThread(RunnableRef& job)
{
   lock();
   {
      // get an idle thread
      PooledThread* t = getIdleThread();
      
      // set job
      t->setJob(job, this);
      
      // unlock thread now that job is assigned
      t->unlock();
      
      // if the thread hasn't started yet, start it
      while(!t->hasStarted())
      {
         if(!t->start(mThreadStackSize))
         {
            // FIXME: thread might not start if maximum threads
            // has been exceeded -- we need a better solution than this:
            
            // yield, try to start again later
            Thread::yield();
         }
      }
   }
   unlock();
}

bool ThreadPool::tryRunJob(Runnable& job)
{
   bool rval = true;
   
   // only try to acquire a permit if infinite threads is not enabled
   if(mThreadSemaphore.getMaxPermitCount() != 0)
   {
      rval = mThreadSemaphore.tryAcquire();
   }
   
   if(rval)
   {
      // run the job on an idle thread
      runJobOnIdleThread(job);
   }
   
   return rval;
}

bool ThreadPool::tryRunJob(RunnableRef& job)
{
   bool rval = true;
   
   // only try to acquire a permit if infinite threads is not enabled
   if(mThreadSemaphore.getMaxPermitCount() != 0)
   {
      rval = mThreadSemaphore.tryAcquire();
   }
   
   if(rval)
   {
      // run the job on an idle thread
      runJobOnIdleThread(job);
   }
   
   return rval;
}

void ThreadPool::runJob(Runnable& job)
{
   // only acquire a permit if infinite threads is not enabled
   bool permitAcquired = true;
   if(mThreadSemaphore.getMaxPermitCount() != 0)
   {
      permitAcquired = mThreadSemaphore.acquire();
   }
   
   if(permitAcquired)
   {
      // run the job on an idle thread
      runJobOnIdleThread(job);
   }
}

void ThreadPool::runJob(RunnableRef& job)
{
   // only acquire a permit if infinite threads is not enabled
   bool permitAcquired = true;
   if(mThreadSemaphore.getMaxPermitCount() != 0)
   {
      permitAcquired = mThreadSemaphore.acquire();
   }
   
   if(permitAcquired)
   {
      // run the job on an idle thread
      runJobOnIdleThread(job);
   }
}

void ThreadPool::jobCompleted(PooledThread* t)
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

void ThreadPool::interruptAllThreads()
{
   lock();
   {
      mListLock.lock();
      {
         // interrupt all threads
         for(ThreadList::iterator i = mThreads.begin();
             i != mThreads.end(); i++)
         {
            (*i)->interrupt();
         }
      }
      mListLock.unlock();
   }
   unlock();
}

void ThreadPool::terminateAllThreads()
{
   lock();
   {
      // interrupt all the threads
      interruptAllThreads();
      
      // join and remove all threads
      for(ThreadList::iterator i = mThreads.begin(); i != mThreads.end();)
      {
         PooledThread* t = *i;
         i = mThreads.erase(i);
         t->join();
         delete t;
      }
      
      // clear threads
      mThreads.clear();
      mIdleThreads.clear();
      
      // clean up expired threads
      cleanupExpiredThreads();
   }
   unlock();
}

void ThreadPool::setPoolSize(unsigned int size)
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

unsigned int ThreadPool::getPoolSize()
{
   return mThreadSemaphore.getMaxPermitCount();
}

void ThreadPool::setThreadStackSize(size_t stackSize)
{
   mThreadStackSize = stackSize;
}

size_t ThreadPool::getThreadStackSize()
{
   return mThreadStackSize;
}

void ThreadPool::setThreadExpireTime(unsigned long long expireTime)
{
   lock();
   {
      mThreadExpireTime = expireTime;
      
      mListLock.lock();
      {
         // update all existing threads
         for(ThreadList::iterator i = mThreads.begin();
             i != mThreads.end(); i++)
         {
            (*i)->setExpireTime(expireTime);
         }
      }
      mListLock.unlock();
   }
   unlock();
}

unsigned long long ThreadPool::getThreadExpireTime()
{
   return mThreadExpireTime;
}

unsigned int ThreadPool::getThreadCount()
{
   return mThreads.size();
}

unsigned int ThreadPool::getRunningThreadCount()
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

unsigned int ThreadPool::getIdleThreadCount()
{
   return mIdleThreads.size();
}
