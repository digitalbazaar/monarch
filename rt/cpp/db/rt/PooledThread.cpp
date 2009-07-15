/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/rt/PooledThread.h"

#include "db/rt/ThreadPool.h"
#include "db/rt/System.h"

using namespace db::rt;

PooledThread::PooledThread(
   ThreadPool* pool, uint32_t expireTime) : Thread(this)
{
   // no job to run yet
   mJob = NULL;
   
   // store thread pool
   mThreadPool = pool;
   
   // sets the expire time for this thread
   setExpireTime(expireTime);
   mExpired = false;
}

PooledThread::~PooledThread()
{
}

void PooledThread::goIdle()
{
   // Note: This method is called inside of this thread's job lock
   uint64_t startTime = System::getCurrentMilliseconds();
   
   // wait on job lock until expire time
   if(mJobLock.wait(getExpireTime()))
   {
      // if this thread has an expire time set and this thread still has
      // no job see if the time has expired
      if(getExpireTime() != 0 && mJob == NULL)
      {
         // check expired time
         uint64_t now = System::getCurrentMilliseconds();
         if((now - startTime) >= getExpireTime())
         {
            // thread must expire
            interrupt();
         }
      }
   }
}

void PooledThread::setJob(Runnable* job)
{
   // Note: The ThreadPool calls this method inside of this thread's
   // job lock when it is not called from this thread itself to prevent
   // issues during idling/unidling
   
   // set job
   mJob = job;
   
   if(job != NULL)
   {
      // notify thread to unidle
      mJobLock.notifyAll();
   }
   else
   {
      // clear job reference
      mJobReference.setNull();
   }
}

void PooledThread::setJob(RunnableRef& job)
{
   // Note: The ThreadPool calls this method inside of this thread's
   // job lock when it is not called from this thread itself to prevent
   // issues during idling/unidling
   
   // set job and reference
   mJob = &(*job);
   mJobReference = job;
   
   // notify thread to unidle
   mJobLock.notifyAll();
}

inline Runnable* PooledThread::getJob()
{
   return mJob;
}

inline ExclusiveLock* PooledThread::getJobLock()
{
   return &mJobLock;
}

void PooledThread::run()
{
   while(!isInterrupted())
   {
      // lock to check for a job
      mJobLock.lock();
      if(mJob != NULL)
      {
         // unlock, run job, notify pool when complete
         mJobLock.unlock();
         mJob->run();
         mThreadPool->jobCompleted(this);
         
         // clear last exception on thread as the job has been completed
         Exception::clear();
      }
      else
      {
         // go idle and then unlock
         goIdle();
         mJobLock.unlock();
      }
   }
   
   // thread expired
   mExpired = true;
}

inline void PooledThread::setExpireTime(uint32_t expireTime)
{
   mExpireTime = expireTime;
}

inline uint32_t PooledThread::getExpireTime()
{
   return mExpireTime;
}

inline bool PooledThread::isExpired()
{
   return mExpired;
}
