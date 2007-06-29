/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "JobThread.h"
#include "System.h"

using namespace std;
using namespace db::rt;

// FIXME: remove iostream include and print outs
#include <iostream>

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
   
   unsigned long long startTime = System::getCurrentMilliseconds();
   
   lock();
   {
      // wait until expire time
      wait(getExpireTime());
   }
   unlock();
   
   // FIXME: check for interruption based on wait's return value
   
   // if this thread has an expire time set and this thread still has
   // no job see if the time has expired
   if(getExpireTime() != 0 && !hasJob())
   {
      // check expired time
      unsigned long long now = System::getCurrentMilliseconds();
      if(now - startTime >= getExpireTime())
      {
         cout << "idle expire time reached, interrupting..." << endl;
         
         // thread must expire
         interrupt();
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
   cout << "JobThread: started." << endl;
   
   try
   {
      while(!isInterrupted())
      {
         // get the Runnable job to run
         Runnable* job = getJob();
         if(job != NULL)
         {
            cout << "JobThread: starting a job..." << endl;
            
            try
            {
               // run job
               job->run();
            }
            catch(Exception& e)
            {
               cout << "exception caught while running job!" << endl;
               cout << "message=" << e.getMessage() << endl;
               cout << "code=" << e.getCode() << endl;
            }
            
            // thread no longer has job
            setJob(NULL);
            
            cout << "JobThread: finished a job." << endl;
         }
         
         if(!isInterrupted())
         {
            // go idle
            goIdle();
         }
      }
      
      if(isInterrupted())
      {
         cout << "JobThread: interrupted." << endl;
      }
   }
   catch(Exception& e)
   {
      cout << "exception caught on JobThread!" << endl;
      cout << "message=" << e.getMessage() << endl;
      cout << "code=" << e.getCode() << endl;
   }
   
   cout << "JobThread terminated." << endl;
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
