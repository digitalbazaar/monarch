/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/JobThread.h"
#include "db/rt/JobThreadPool.h"
#include "db/rt/System.h"

using namespace db::rt;

JobThread::JobThread(unsigned long long expireTime) : Thread(this)
{
   // no job to run yet
   mJob = NULL;
   mThreadPool = NULL;
   
   // sets the expire time for this thread
   setExpireTime(expireTime);
   mExpired = false;
}

JobThread::~JobThread()
{
}

void JobThread::goIdle()
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

void JobThread::setJob(Runnable* job, JobThreadPool* pool)
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

void JobThread::setJob(CollectableRunnable& job, JobThreadPool* pool)
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

Runnable* JobThread::getJob()
{
   return mJob;
}

void JobThread::run()
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

bool JobThread::hasJob()
{
   bool rval = false;
   
   lock();
   {
      rval = (mJob != NULL);
   }
   unlock();
   
   return rval;
}

void JobThread::setExpireTime(unsigned long long expireTime)
{
   mExpireTime = expireTime;
}

unsigned long long JobThread::getExpireTime()
{
   return mExpireTime;
}

bool JobThread::isExpired()
{
   bool rval = false;
   
   lock();
   {
      rval = mExpired;
   }
   unlock();
   
   return rval;
}
