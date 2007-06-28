/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Monitor.h"
#include "GetTimeOfDay.h"
#include "Thread.h"

using namespace std;
using namespace db::rt;

Monitor::Monitor()
{
   // initialize mutex
   pthread_mutex_init(&mMutex, NULL);
   
   // initialize conditional
   pthread_cond_init(&mWaitCondition, NULL);
   
   // threads must wait on the wait condition
   mMustWait = true;
   
   // no current thread or locks yet
   mCurrentThread = NULL;
   mLockCount = 0;
}

Monitor::~Monitor()
{
   // destroy mutex
   pthread_mutex_destroy(&mMutex);
   
   // destroy conditional
   pthread_cond_destroy(&mWaitCondition);
}

void Monitor::enter()
{
   // threads must wait
   mMustWait = true;
   
   Thread* t = Thread::currentThread();
   if(t != NULL)
   {
      // ensure the current thread isn't already in the monitor
      if(mCurrentThread != t)
      {
         // lock this monitor's mutex
         pthread_mutex_lock(&mMutex);
         
         // set the current thread and lock count
         mCurrentThread = t;
         mLockCount = 1;
      }
      else
      {
         // increment lock count
         mLockCount++;
      }
   }
}

void Monitor::exit()
{
   // ensure the current thread is in the monitor
   Thread* t = Thread::currentThread();
   if(t != NULL && mCurrentThread == t)
   {
      if(mLockCount == 1)
      {
         // clear the current thread and lock count
         mCurrentThread = NULL;
         mLockCount = 0;
         
         // unlock this monitor's mutex
         pthread_mutex_unlock(&mMutex);
      }
      else
      {
         // decrement lock count
         mLockCount--;
      }
   }
}

void Monitor::wait(unsigned long timeout)
{
   // ensure the current thread is in the monitor
   Thread* t = Thread::currentThread();
   if(t != NULL && mCurrentThread == t)
   {
      // clear the thread from the monitor and store old lock count
      mCurrentThread = NULL;
      unsigned int lockCount = mLockCount;
      mLockCount = 0;
      
      if(timeout == 0)
      {
         // wait indefinitely on the wait condition
         pthread_cond_wait(&mWaitCondition, &mMutex);
      }
      else
      {
         // determine seconds and nanoseconds (timeout is in milliseconds and
         // 1000 milliseconds = 1 second = 1000000 nanoseconds
         unsigned long secs = timeout / 1000UL;
         unsigned long nsecs = timeout % 1000UL * 1000000UL;
         
         struct timeval now;
         struct timespec timeout;
         gettimeofday(&now, NULL);
         
         // add timeout to current time (1 microsecond = 1000 nanoseconds)
         timeout.tv_sec = now.tv_sec + secs;
         timeout.tv_nsec = now.tv_usec * 1000UL + nsecs;
         
         // do timed wait
         int rc = pthread_cond_timedwait(&mWaitCondition, &mMutex, &timeout);
         if(rc == ETIMEDOUT)
         {
            // timeout reached
         }
      }
      
      // restore current thread and lock count
      mCurrentThread = t;
      mLockCount = lockCount;
   }
}

void Monitor::notify()
{
   // threads do not need to wait again when they wake up
   mMustWait = false;
   
   // signal a thread locked on the conditional to wake up
   pthread_cond_signal(&mWaitCondition);
}

void Monitor::notifyAll()
{
   // threads do not need to wait again when they wake up
   mMustWait = false;
   
   // signal all threads to wake up
   signalAll();
}

void Monitor::signalAll()
{
   // signal all threads locked on the conditional to wake up
   pthread_cond_broadcast(&mWaitCondition);
}

bool Monitor::mustWait()
{
   return mMustWait;
}
