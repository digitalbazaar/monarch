/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "JobThread.h"
#include "System.h"

using namespace db::rt;

JobThread::JobThread(unsigned long long expireTime) : Thread(this)
{
   // no Runnable job to run yet
   mJob = NULL;
   mSemaphore = NULL;
   mPermits = 0;
   
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

void JobThread::setJob(Runnable* job, Semaphore* semaphore, int permits)
{
   lock();
   {
      // set job, semaphore, and permits
      mJob = job;
      mSemaphore = semaphore;
      mPermits = permits;
      
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
      // get the Runnable job to run
      Runnable* job = mJob;
      if(job != NULL)
      {
         // run job
         job->run();
         
         lock();
         {
            // release permits for job
            if(mSemaphore != NULL)
            {
               mSemaphore->release(mPermits);
            }
            
            // thread no longer has job
            setJob(NULL, NULL, 0);
         }
         unlock();
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

bool JobThread::isIdle()
{
   return !hasJob();
}

void JobThread::setExpireTime(unsigned long long expireTime)
{
   mExpireTime = expireTime;
}

unsigned long long JobThread::getExpireTime()
{
   return mExpireTime;
}
