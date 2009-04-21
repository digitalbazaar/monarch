/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/rt/SharedLock.h"

using namespace db::rt;

/*
 Note: On Windows & Mac OS only, it is possible for the current
 thread to hold the shared lock but be unable to increment the
 shared lock count because another thread is waiting to lock
 exclusively. Linux systems will correctly increment the shared lock
 count because the lock is already held by the current thread --
 thereby avoiding the nasty deadlock issue.
 
 The POSIX spec states that a thread that holds the shared lock can
 successfully call pthread_rwlock_rdlock() n times so long as it
 also calls pthread_rwlock_unlock() the same number of times. The
 spec, however, also states that a shared lock cannot be acquired
 if a thread is waiting to acquire the exclusive lock. This
 introduces some confusion into how that line is interpreted: we
 can either assume that a thread that has already acquired the lock
 doesn't need to "acquire" it again (Linux interpretation), or we
 can assume that it does (Windows + Mac OS). However, if we choose
 the later interpretation, recursive shared locks make little to no
 sense, causing them to get into potentially countless deadlock
 scenarios that are logistical nightmares to code around.
 
 Here, if we are on Linux, we simply use a pthread_rwlock. If we are
 on Windows or Mac OS, we provide a custom implementation using mutexes
 and wait conditions to provide correctness w/respect to recursive shared
 locks.
 */

#if defined(WIN32) && defined(MACOS)
// windows & macos:
SharedLock::SharedLock() :
   mThreadId(Thread::getInvalidThreadId()),
   mSharedCount(0),
   mExclusiveCount(0),
   mExclusiveRequests(0),
   mAllowShared(false)
{
   // create mutex attributes
   pthread_mutexattr_t mutexAttr;
   pthread_mutexattr_init(&mutexAttr);
   
   // use fastest type of mutex
   pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_NORMAL);
   
   // initialize mutex
   pthread_mutex_init(&mMutex, &mutexAttr);
   
   // initialize wait conditions
   pthread_cond_init(&mSharedCondition, NULL);
   pthread_cond_init(&mExclusiveCondition, NULL);
   
   // destroy mutex attributes
   pthread_mutexattr_destroy(&mutexAttr);
}

SharedLock::~SharedLock()
{
   // destroy mutex
   pthread_mutex_destroy(&mMutex);
   
   // destroy wait conditionals
   pthread_cond_destroy(&mSharedCondition);
   pthread_cond_destroy(&mExclusiveCondition);
}

void SharedLock::lockShared()
{
   // see if this thread holds the exclusive lock
   int rc = pthread_equal(mThreadId, pthread_self());
   if(rc == 0)
   {
      // this thread does not have the exclusive lock:
      
      // enter critical section
      pthread_mutex_lock(&mMutex);
      
      // wait while exclusive lock count above 0 or
      // while exclusive lock are waiting AND no
      // shared lock should be allowed to proceed
      // (helps to prevent "writer" starvation)
      while(mExclusiveCount > 0 || (mExclusiveRequests > 0 && !mAllowShared))
      {
         pthread_cond_wait(&mSharedCondition, &mMutex);
      }
      
      // shared lock acquired
      mSharedCount++;
      
      // do not allow another shared lock if there are exclusive locks waiting
      mAllowShared = false;
      
      // exit critical section
      pthread_mutex_unlock(&mMutex);
   }
   else
   {
      // this thread has the exclusive lock, so increase shared lock count
      mSharedCount++;
   }
}

void SharedLock::unlockShared()
{
   // enter critical section
   pthread_mutex_lock(&mMutex);
   
   // shared lock released
   mSharedCount--;
   
   if(mExclusiveCount == 0 && mSharedCount == 0)
   {
      // notify threads waiting on exclusive locks
      // (shared lock threads are not blocked and do not need notification)
      pthread_cond_broadcast(&mExclusiveCondition);
   }
   
   // exit critical section
   pthread_mutex_unlock(&mMutex);
}

void SharedLock::lockExclusive()
{
   // see if this thread holds the exclusive lock
   pthread_t self = pthread_self();
   int rc = pthread_equal(mThreadId, self);
   if(rc == 0)
   {
      // this thread does not have the exclusive lock:
      
      // enter critical section
      pthread_mutex_lock(&mMutex);
      
      // exclusive lock requested
      mExclusiveRequests++;
      
      // wait for exclusive and shared lock counts to hit 0
      while(mExclusiveCount > 0 && mSharedCount > 0)
      {
         pthread_cond_wait(&mExclusiveCondition, &mMutex);
      }
      
      // exclusive lock acquired
      mExclusiveCount++;
      mExclusiveRequests--;
      
      // set thread that holds the exclusive lock
      mThreadId = self;
      
      // exit critical section
      pthread_mutex_unlock(&mMutex);
   }
   else
   {
      // this thread has the exclusive lock, so increase exclusive lock count
      mExclusiveCount++;
   }
}

void SharedLock::unlockExclusive()
{
   // enter critical section
   pthread_mutex_lock(&mMutex);
   
   // exclusive lock released
   mExclusiveCount--;
   
   if(mExclusiveCount == 0)
   {
      // allow a shared lock if there are exclusive locks waiting since
      // this thread just finished using an exclusive lock
      // (helps prevent "reader" starvation)
      mAllowShared = true;
      
      // thread no longer holds exclusive lock
      mThreadId = Thread::getInvalidThreadId();
      
      // notify threads waiting on shared locks first since
      // an exclusive lock was just released
      pthread_cond_broadcast(&mSharedCondition);
      pthread_cond_broadcast(&mExclusiveCondition);
   }
   
   // exit critical section
   pthread_mutex_unlock(&mMutex);
}

#else
// non-windows & non-macos:
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

#endif // non-windows & non-macos
