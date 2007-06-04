/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Object.h"
#include "Exception.h"
#include "InterruptedException.h"
#include "Thread.h"

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
      // FIXME: implement timeout
      throw Exception("Object::wait(timeout) not implemented yet!");
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
