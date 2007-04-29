/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Object.h"

using namespace std;
using namespace db::system;

Object::Object()
{
   // initialize mutex
   pthread_mutex_init(&mMutex, NULL);
   
   // initialize conditional
   pthread_cond_init(&mWaitCondition, NULL);
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
   // lock this Object's mutex
   pthread_mutex_lock(&mMutex);
}

void Object::unlock()
{
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

void Object::wait(unsigned long time)
{
   // wait on this Object's wait condition
   pthread_cond_wait(&mWaitCondition, &mMutex);
}

bool Object::operator==(const Object &rhs) const
{
   return equals(rhs);
}
