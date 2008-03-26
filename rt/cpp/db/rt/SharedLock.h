/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_SharedLock_H
#define db_rt_SharedLock_H

#include "db/rt/Thread.h"

namespace db
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
 * by the same thread that holds the exclusive lock.
 * 
 * @author Dave Longley
 */
class SharedLock
{
protected:
   /**
    * The lock for checking the shared/exclusive status.
    */
   Object mStatusLock;
   
   /**
    * The current number of shared locks.
    */
   int mShared;
   
   /**
    * The current number of exclusive locks (which can only be more than one
    * if a particular thread is recursively locked).
    */
   int mExclusive;
   
   /**
    * The thread that holds the exclusive lock.
    */
   Thread* mThread;
   
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
} // end namespace db
#endif
