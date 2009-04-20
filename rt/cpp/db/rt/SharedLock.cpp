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
   int rc = pthread_equal(mThreadId, pthread_self());
   if(rc == 0)
   {
      // Note: On Windows & Mac OS only, it is possible for the current
      // thread to hold the shared lock but be unable to increment the
      // shared lock count because another thread is waiting to lock
      // exclusively. Linux systems will correctly increment the shared lock
      // count because the lock is already held by the current thread --
      // thereby avoiding the nasty deadlock issue.
      // 
      // The POSIX spec states that a thread that holds the shared lock can
      // successfully call pthread_rwlock_rdlock() n times so long as it
      // also calls pthread_rwlock_unlock() the same number of times. The
      // spec, however, also states that a shared lock cannot be acquired
      // if a thread is waiting to acquire the exclusive lock. This
      // introduces some confusion into how that line is interpreted: we
      // can either assume that a thread that has already acquired the lock
      // doesn't need to "acquire" it again (Linux interpretation), or we
      // can assume that it does (Windows + Mac OS). However, if we choose
      // the later interpretation, recursive shared locks make little to no
      // sense, causing them to get into potentially countless deadlock
      // scenarios that are logistical nightmares to code around.
      // 
      // Here, if we are on Linux, we simply call pthread_rwlock_rdlock().
      // If we are on Windows or Mac OS, then we do pthread_rwlock_tryrdlock()
      // first, then check for EDEADLK, and if we have found it, we unlock
      // the shared lock and then re-lock it. This adds a caveat to any
      // code that is using recursive shared locks ... they may lose their
      // lock temporarily when they recurse. This is the best compromise that
      // could be thought of without re-implementing the lock entirely or
      // stating that recursive shared locks are a programming error and in
      // breach of the programming contract with this class.
      
#if defined(WIN32) || defined(MACOS)
      // try to obtain a shared lock
      if(pthread_rwlock_tryrdlock(&mLock) != 0)
      {
         if(errno == EDEADLK)
         {
            // this thread already has the shared lock, so unlock and then
            // relock it to proceed
            pthread_rwlock_unlock(&mLock);
            pthread_rwlock_rdlock(&mLock);
         }
         else if(errno == EBUSY)
         {
            // lock was simply busy, so do a regular shared lock
            pthread_rwlock_rdlock(&mLock);
         }
      }
#else
      // Linux: simply obtain a shared lock
      pthread_rwlock_rdlock(&mLock);
#endif
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
   int rc = pthread_equal(mThreadId, pthread_self());
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
