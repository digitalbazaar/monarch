/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Monitor.h"
#include "GetTimeOfDay.h"

using namespace std;
using namespace db::rt;

Monitor::Monitor()
{
   // initialize mutex attributes
   pthread_mutexattr_init(&mMutexAttributes);
   pthread_mutexattr_settype(&mMutexAttributes, PTHREAD_MUTEX_RECURSIVE);
   
   // initialize mutex
   pthread_mutex_init(&mMutex, &mMutexAttributes);
   
   // initialize conditional
   pthread_cond_init(&mWaitCondition, NULL);
   
   // no locks yet
   mLockCount = 0;
}

Monitor::~Monitor()
{
   // destroy mutex
   pthread_mutex_destroy(&mMutex);
   
   // destroy mutex attributes
   pthread_mutexattr_destroy(&mMutexAttributes);
   
   // destroy conditional
   pthread_cond_destroy(&mWaitCondition);
}

void Monitor::enter()
{
   // lock this monitor's mutex
   pthread_mutex_lock(&mMutex);
   
   // increment lock count
   mLockCount++;
}

void Monitor::exit()
{
   // decrement lock count
   mLockCount--;
   
   // unlock this monitor's mutex
   pthread_mutex_unlock(&mMutex);
}

void Monitor::wait(unsigned long timeout)
{
   // store old lock count
   unsigned int lockCount = mLockCount;
   
   // decrement lock count until 1
   while(mLockCount > 1)
   {
      exit();
   }
   
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
   
   // increment lock count until old lock count is reached
   while(mLockCount < lockCount)
   {
      enter();
   }
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
