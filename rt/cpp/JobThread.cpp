/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "JobThread.h"
#include "System.h"

using namespace std;
using namespace db::rt;

JobThread::JobThread(unsigned long long expireTime) : Thread(this)
{
   // no Runnable job to run yet
   mJob = NULL;
   
   // sets the expire time for this thread
   setExpireTime(expireTime);
}

JobThread::~JobThread()
{
}

Runnable* JobThread::getJob()
{
   return mJob;
}

void JobThread::goIdle()
{
   // set thread name
   setName("JobThread: idle");
   
   InterruptedException* e = NULL;
   unsigned long long startTime = System::getCurrentMilliseconds();
   
   lock();
   {
      // wait until expire time
      e = wait(getExpireTime());
   }
   unlock();
   
   if(e == NULL && !isInterrupted())
   {
      // if this thread has an expire time set and this thread still has
      // no job see if the time has expired
      if(getExpireTime() != 0 && !hasJob())
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

void JobThread::wakeup()
{
   lock();
   {
      // notify thread to stop waiting
      notify();
   }
   unlock();
}

void JobThread::setJob(Runnable* job)
{
   // set job
   mJob = job;
   
   if(job != NULL)
   {
      // set thread name
      string str;
      setName("JobThread: running job '" + getJob()->toString(str) + "'");
      
      // wake up thread
      wakeup();
   }
   else
   {
      // set thread name
      setName("JobThread: no job");
   }
}

void JobThread::run()
{
   while(!isInterrupted())
   {
      // get the Runnable job to run
      Runnable* job = getJob();
      if(job != NULL)
      {
         // run job
         job->run();
         
         // thread no longer has job
         setJob(NULL);
      }
      
      if(!isInterrupted())
      {
         // go idle
         goIdle();
      }
   }
}

bool JobThread::hasJob()
{
   return getJob() != NULL;
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
