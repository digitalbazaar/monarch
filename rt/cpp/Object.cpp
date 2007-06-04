/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Object.h"
#include "Exception.h"
#include "InterruptedException.h"
#include "Thread.h"
#include "GetTimeOfDay.h"

using namespace std;
using namespace db::rt;

Object::Object()
{
   // initialize mutex
   pthread_mutex_init(&mMutex, NULL);
   
   // initialize conditional
   pthread_cond_init(&mWaitCondition, NULL);
   
   // no lock owner yet
   mLockOwner = NULL;
}

Object::~Object()
{
   // destroy mutex
   pthread_mutex_destroy(&mMutex);
   
   // destroy conditional
   pthread_cond_destroy(&mWaitCondition);
}

bool Object::equals(const Object& obj) const
{
   bool rval = false;
   
   // compare memory locations
   if(this == &obj)
   {
      rval = true;
   }
   
   return rval;
}

void Object::lock()
{
   // get the current thread
   Thread* t = Thread::currentThread();
   
   // try to lock this Object's mutex
   int rc = pthread_mutex_trylock(&mMutex);
   if(rc != 0)
   {
      // see if this thread already owns the lock
      if(t != mLockOwner)
      {
         // thread doesn't own the lock, so lock this Object's mutex
         pthread_mutex_lock(&mMutex);
      }
   }
   
   // set the lock owner
   mLockOwner = t;
}

void Object::unlock()
{
   // clear the lock owner
   mLockOwner = NULL;
   
   // unlock this Object's mutex
   pthread_mutex_unlock(&mMutex);
}

void Object::notify()
{
   // signal a thread locked on the conditional to wake up
   pthread_cond_signal(&mWaitCondition);
}

void Object::notifyAll()
{
   // signal all threads locked on the conditional to wake up
   pthread_cond_broadcast(&mWaitCondition);
}

void Object::wait(unsigned long timeout)
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

bool Object::operator==(const Object &rhs) const
{
   return equals(rhs);
}

string& Object::toString(string& str)
{
   str = "Object";
   return str;
}
