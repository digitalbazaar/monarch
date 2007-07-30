/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "JobThread.h"
#include "JobThreadPool.h"
#include "System.h"

using namespace db::rt;

JobThread::JobThread(unsigned long long expireTime) : Thread(this)
{
   // no job to run yet
   mJob = NULL;
   mThreadPool = NULL;
   
   // sets the expire time for this thread
   setExpireTime(expireTime);
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
   }
   unlock();
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
