/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Monitor.h"
#include "GetTimeOfDay.h"

using namespace std;
using namespace db::rt;

Monitor::Monitor()
{
   // create mutex attributes
   pthread_mutexattr_t mutexAttr;
   pthread_mutexattr_init(&mutexAttr);
   
   // use fastest type of mutex
   pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_NORMAL);
   
   // initialize mutex
   pthread_mutex_init(&mMutex, &mutexAttr);
   
   // initialize wait conditional
   pthread_cond_init(&mWaitCondition, NULL);
   
   // destroy mutex attributes
   pthread_mutexattr_destroy(&mutexAttr);
   
   // no locks yet
   mThreadId = pthread_self();
   mHasThread = false;
   mLockCount = 0;
}

Monitor::~Monitor()
{
   // destroy mutex
   pthread_mutex_destroy(&mMutex);
   
   // destroy wait conditional
   pthread_cond_destroy(&mWaitCondition);
}

void Monitor::enter()
{
   // see if this thread isn't already in this monitor
   pthread_t self = pthread_self();
   int rc = pthread_equal(mThreadId, self);
   if(rc == 0 || !mHasThread)
   {
      // lock this monitor's mutex
      pthread_mutex_lock(&mMutex);
      
      // set thread that is in this monitor
      mHasThread = true;
      mThreadId = self;
   }
   
   // increment lock count
   mLockCount++;
}

void Monitor::exit()
{
   // decrement lock count
   mLockCount--;
   
   if(mLockCount == 0)
   {
      // no longer a thread in this monitor
      mHasThread = false;
      
      // unlock this monitor's mutex
      pthread_mutex_unlock(&mMutex);
   }
}

void Monitor::wait(unsigned long timeout)
{
   // store old thread and lock count
   pthread_t threadId = mThreadId;
   unsigned int lockCount = mLockCount;
   
   // reset thread and lock count
   mHasThread = false;
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
      struct timespec to;
      gettimeofday(&now, NULL);
      
      // add timeout to current time (1 microsecond = 1000 nanoseconds)
      to.tv_sec = now.tv_sec + secs;
      to.tv_nsec = now.tv_usec * 1000UL + nsecs;
      
      // do timed wait
      int rc = pthread_cond_timedwait(&mWaitCondition, &mMutex, &to);
      if(rc == ETIMEDOUT)
      {
         // timeout reached
      }
   }
   
   // restore old thread and lock count
   mThreadId = threadId;
   mHasThread = true;
   mLockCount = lockCount;
}

void Monitor::notify()
{
   // signal a thread locked on the conditional to wake up
   pthread_cond_signal(&mWaitCondition);
}

void Monitor::notifyAll()
{
   // signal all threads to wake up
   signalAll();
}

void Monitor::signalAll()
{
   // signal all threads locked on the conditional to wake up
   pthread_cond_broadcast(&mWaitCondition);
}
