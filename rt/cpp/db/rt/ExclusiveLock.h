/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_ExclusiveLock_H
#define db_rt_ExclusiveLock_H

#include "db/rt/Monitor.h"

namespace db
{
namespace rt
{

/**
 * An ExclusiveLock is used to sychronize threads in a multi-threaded system.
 * 
 * ExclusiveLocks are recursive, such that if a particular thread has acquired
 * a particular lock, it can call lock() multiple times thereafter without
 * causing deadlock or an indeterminant state. Of course, unlock() must be
 * called an equal number of times to release the lock.
 * 
 * @author Dave Longley
 */
class ExclusiveLock
{
protected:
   /**
    * The Monitor for this ExclusiveLock.
    */
   Monitor mMonitor;
   
public:
   /**
    * Constructs a new ExclusiveLock.
    */
   ExclusiveLock();
   
   /**
    * Destructs this ExclusiveLock.
    */
   virtual ~ExclusiveLock();
   
   /**
    * Locks this ExclusiveLock for the current thread. This method will block
    * until the lock can be acquired.
    */
   virtual void lock();
   
   /**
    * Unlocks this ExclusiveLock.
    */
   virtual void unlock();
   
   /**
    * Notifies one thread (any thread) that is waiting on this lock to wake up.
    * 
    * This lock must be locked by calling lock() before executing this
    * method. It must be unlocked after executing it.
    */
   virtual void notify();
   
   /**
    * Notifies all threads that are waiting on this lock to wake up.
    * 
    * This lock must be locked by calling lock() before executing this
    * method. It must be unlocked after executing it.
    */
   virtual void notifyAll();
   
   /**
    * Causes the current thread to wait until another thread calls
    * notify() or notifyAll() on this lock or until the passed number
    * of milliseconds pass.
    *
    * This lock must be locked by calling lock() before executing this
    * method. It must be unlocked after executing it.
    *
    * @param timeout the number of milliseconds to wait for a notify call
    *                before timing out, 0 to wait indefinitely.
    * 
    * @return false if the current thread is interrupted while waiting (with
    *         an interrupted exception set), true if not.
    */
   virtual bool wait(uint32_t timeout = 0);
};

} // end namespace rt
} // end namespace db

#endif
