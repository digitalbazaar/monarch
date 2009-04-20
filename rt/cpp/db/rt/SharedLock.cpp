/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/SharedLock.h"

using namespace db::rt;

SharedLock::SharedLock()
{
   // initialize lock
   pthread_rwlock_init(&mLock, NULL);
   
   // no locks yet
   mThreadId = Thread::getInvalidThreadId();
   mLockCount = 0;
}

SharedLock::~SharedLock()
{
   // destroy lock
   pthread_rwlock_destroy(&mLock);
}

void SharedLock::lockShared()
{
   // see if this thread holds the exclusive lock
   pthread_t self = pthread_self();
   int rc = pthread_equal(mThreadId, self);
   if(rc == 0)
   {
      // obtain a shared lock
      pthread_rwlock_rdlock(&mLock);
   }
   else
   {
      // current thread has the exclusive lock, so bump up lock count
      mLockCount++;
   }
}

void SharedLock::unlockShared()
{
   // see if this thread holds the exclusive lock
   pthread_t self = pthread_self();
   int rc = pthread_equal(mThreadId, self);
   if(rc == 0)
   {
      // release shared lock
      pthread_rwlock_unlock(&mLock);
   }
   else
   {
      // release exclusive lock
      unlockExclusive();
   }
}

void SharedLock::lockExclusive()
{
   // see if this thread holds the exclusive lock
   pthread_t self = pthread_self();
   int rc = pthread_equal(mThreadId, self);
   if(rc == 0)
   {
      // obtain the exclusive lock
      pthread_rwlock_wrlock(&mLock);
      
      // set thread that holds the exclusive lock
      mThreadId = self;
   }
   
   // increment lock count
   mLockCount++;   
}

void SharedLock::unlockExclusive()
{
   // decrement lock count
   mLockCount--;
   
   if(mLockCount == 0)
   {
      // thread no longer holds exclusive lock
      mThreadId = Thread::getInvalidThreadId();
      
      // release exclusive lock
      pthread_rwlock_unlock(&mLock);
   }
}
