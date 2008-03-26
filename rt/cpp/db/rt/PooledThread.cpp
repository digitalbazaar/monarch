/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/PooledThread.h"
#include "db/rt/ThreadPool.h"
#include "db/rt/System.h"

using namespace db::rt;

PooledThread::PooledThread(
   ThreadPool* pool, unsigned long long expireTime) : Thread(this)
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
   unsigned long long startTime = System::getCurrentMilliseconds();
   
   // wait until expire time
   if(wait(getExpireTime()))
   {
      // if this thread has an expire time set and this thread still has
      // no job see if the time has expired
      if(getExpireTime() != 0 && mJob == NULL)
      {
         // check expired time
         unsigned long long now = System::getCurrentMilliseconds();
         if(now - startTime >= getExpireTime())
         {
            // thread must expire
            interrupt();
         }
      }
   }
}

void PooledThread::setJob(Runnable* job)
{
   lock();
   {
      // set job
      mJob = job;
      
      if(job != NULL)
      {
         // notify thread to stop waiting
         notifyAll();
      }
      else
      {
         // clear job reference
         mJobReference.setNull();
      }
   }
   unlock();
}

void PooledThread::setJob(RunnableRef& job)
{
   lock();
   {
      // set job and reference
      mJob = &(*job);
      mJobReference = job;
      
      // notify thread to stop waiting
      notifyAll();
   }
   unlock();
}

Runnable* PooledThread::getJob()
{
   return mJob;
}

void PooledThread::run()
{
   while(!isInterrupted())
   {
      // lock to check for a job
      lock();
      if(mJob != NULL)
      {
         // unlock, run job, notify pool when complete
         unlock();
         mJob->run();
         mThreadPool->jobCompleted(this);
      }
      else
      {
         // go idle and then unlock
         goIdle();
         unlock();
      }
   }
   
   // thread expired
   mExpired = true;
}

void PooledThread::setExpireTime(unsigned long long expireTime)
{
   mExpireTime = expireTime;
}

unsigned long long PooledThread::getExpireTime()
{
   return mExpireTime;
}

bool PooledThread::isExpired()
{
   return mExpired;
}
