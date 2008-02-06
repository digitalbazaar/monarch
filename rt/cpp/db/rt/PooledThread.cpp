/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/PooledThread.h"
#include "db/rt/ThreadPool.h"
#include "db/rt/System.h"

using namespace db::rt;

PooledThread::PooledThread(unsigned long long expireTime) : Thread(this)
{
   // no job to run yet
   mJob = NULL;
   mThreadPool = NULL;
   
   // sets the expire time for this thread
   setExpireTime(expireTime);
   mExpired = false;
}

PooledThread::~PooledThread()
{
}

void PooledThread::goIdle()
{
   lock();
   {
      if(mJob == NULL)
      {
         unsigned long long startTime = System::getCurrentMilliseconds();
         
         // wait until expire time
         if(wait(getExpireTime()) == NULL)
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
                  mExpired = true;
                  interrupt();
               }
            }
         }
      }
   }
   unlock();
}

void PooledThread::setJob(Runnable* job, ThreadPool* pool)
{
   lock();
   {
      // set job and pool
      mJob = job;
      mThreadPool = pool;
      
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

void PooledThread::setJob(RunnableRef& job, ThreadPool* pool)
{
   lock();
   {
      // set job, reference, and pool
      mJob = &(*job);
      mJobReference = job;
      mThreadPool = pool;
      
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
      if(hasJob())
      {
         // run job
         mJob->run();
         
         // notify pool that job is complete
         if(mThreadPool != NULL)
         {
            mThreadPool->jobCompleted(this);
         }
      }
      else
      {
         // go idle
         goIdle();
      }
   }
}

bool PooledThread::hasJob()
{
   bool rval = false;
   
   lock();
   {
      rval = (mJob != NULL);
   }
   unlock();
   
   return rval;
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
   bool rval = false;
   
   lock();
   {
      rval = mExpired;
   }
   unlock();
   
   return rval;
}
