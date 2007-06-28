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
   
   // no current thread yet
   mCurrentThread = NULL;   
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
   
   // get the current thread
   Thread* t = Thread::currentThread();
   if(t != NULL)
   {
      // try to lock this monitor's mutex
      int rc = pthread_mutex_trylock(&mMutex);
      if(rc != 0)
      {
         // see if this thread is already in this monitor
         if(t != mCurrentThread)
         {
            // thread isn't in the monitor, so lock this monitor's mutex
            pthread_mutex_lock(&mMutex);
         }
      }
      
      // set the current thread
      mCurrentThread = t;
   }
}

void Monitor::exit()
{
   if(mCurrentThread != NULL)
   {
      // clear the current thread
      mCurrentThread = NULL;
      
      // unlock this monitor's mutex
      pthread_mutex_unlock(&mMutex);
   }
}

void Monitor::wait(unsigned long timeout)
{
   if(timeout == 0)
   {
      // wait indefinitely on this Object's wait condition
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
