/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
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
    * To avoid race conditions, this lock must be locked by calling lock()
    * before executing this method and unlocked after executing it.
    *
    * Note: The thread may wake up due to some operating system level activity
    * prior to the timeout and prior to any actual notify call. This should
    * be checked for by using wait() inside of a conditional loop. As an
    * alternative, wait(timeout, condition, stop) may be called as a simple
    * implementation of this paradigm.
    *
    * @param timeout the number of milliseconds to wait for a notify call
    *                before timing out, 0 to wait indefinitely.
    *
    * @return false if the current thread is interrupted while waiting (with
    *         an interrupted exception set), true if not.
    */
   virtual bool wait(uint32_t timeout = 0);

   /**
    * Causes the current thread to wait until the specified time elapses,
    * the thread is interrupted, or until the passed condition is set to
    * the passed boolean value. The condition will only be examined if notify()
    * or notifyAll() is called or if the operating system wakes up the thread
    * for some reason.
    *
    * To avoid race conditions, this lock must be locked by calling lock()
    * before executing this method and unlocked after executing it.
    *
    * The usage for this method is:
    *
    * ExclusiveLock lock;
    * uint32_t time = 500;   // 500 milliseconds
    * bool workToDo = false; // we are waiting for work or a timeout
    *
    * // lock
    * lock.lock();
    *
    * // do something if desired
    *
    * // wait until specified time or until there is work to do, if notified
    * lock.wait(time, &workToDo, true);
    *
    * // do something else if desired
    *
    * // unlock
    * lock.unlock();
    *
    * @param timeout the number of milliseconds to wait before timing out,
    *                0 to wait indefinitely, updated to remaining time if
    *                timeout does not fully elapse before returning.
    * @param condition the condition to check.
    * @param stop the value the condition must be to stop waiting before the
    *             timeout expires, if notify() or notifyAll() has been called.
    *
    * @return true if the thread was not interrupted, false if it was.
    */
   virtual bool wait(uint32_t& timeout, bool* condition, bool stop);
};

} // end namespace rt
} // end namespace db

#endif
