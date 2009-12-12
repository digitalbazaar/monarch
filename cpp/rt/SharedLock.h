/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_rt_SharedLock_H
#define monarch_rt_SharedLock_H

#include "monarch/rt/Thread.h"

namespace monarch
{
namespace rt
{

/**
 * A SharedLock is used to provide a more fine-grained thread lock that
 * differentiates between shared locking and exclusive locking. This can
 * be used to increase performance when certain blocks of critical code
 * can share the same lock whereas only select others require exclusive
 * an lock. For example, shared locks can be used in many "read" scenarios,
 * and exclusive locks can be used in many "write" scenarios.
 *
 * When a shared lock is engaged, only other shared locks will be able to
 * gain access. When an exclusive lock is engaged, only the code inside of
 * that lock will execute. In order for an exclusive lock to be obtained,
 * all shared locks must first be released.
 *
 * If an exclusive lock is engaged, then shared locks can only be obtained
 * by the same thread that holds the exclusive lock. Only once the exclusive
 * lock and all of the shared locks that were obtained inside of it are
 * released will other threads be able to obtain new locks. In other words,
 * if an exclusive lock is held, new shared locks on the same thread act
 * as if they are simply recursing the exclusive lock.
 *
 * Note: This SharedLock assumes that no thread will be assigned an ID of 0.
 * If a thread is, then there is a race condition that could result in that
 * thread obtaining a lock when it isn't really inside of this Monitor.
 *
 * monarch::rt::Thread disallows threads from being created with an ID of 0.
 *
 * @author Dave Longley
 */
class SharedLock
{
protected:
#if defined(WIN32) || defined(MACOS)
// windows & macos:
   /**
    * The mutex for this lock.
    */
   pthread_mutex_t mMutex;

   /**
    * The condition used to wait and signal shared lock threads.
    */
   pthread_cond_t mSharedCondition;

   /**
    * The condition used to wait and signal exclusive lock threads.
    */
   pthread_cond_t mExclusiveCondition;

   /**
    * Stores the pthread ID of the thread that currently holds the
    * exclusive lock.
    */
   pthread_t mThreadId;

   /**
    * A counter for the number of shared locks held by all threads.
    */
   uint32_t mSharedCount;

   /**
    * A counter for the number of recursive locks held by the thread that
    * holds an exclusive lock.
    */
   uint32_t mExclusiveCount;

   /**
    * A counter for the number of threads that have requested an exclusive lock.
    */
   uint32_t mExclusiveRequests;
#else
// non-windows & non-macos:
   /**
    * The pthread read/write lock.
    */
   pthread_rwlock_t mLock;

   /**
    * Stores the pthread ID of the thread that currently holds the
    * exclusive lock.
    */
   pthread_t mThreadId;

   /**
    * A counter for the number of requested locks (shared or exclusive) by the
    * thread that holds an exclusive lock.
    */
   unsigned int mLockCount;
#endif

public:
   /**
    * Constructs a new SharedLock.
    */
   SharedLock();

   /**
    * Destructs this SharedLock.
    */
   virtual ~SharedLock();

   /**
    * Acquires a shared lock.
    */
   void lockShared();

   /**
    * Releases a shared lock.
    */
   void unlockShared();

   /**
    * Acquires an exclusive lock.
    */
   void lockExclusive();

   /**
    * Releases an exclusive lock.
    */
   void unlockExclusive();
};

} // end namespace rt
} // end namespace monarch
#endif
