/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_rt_Monitor_H
#define db_rt_Monitor_H

#include <pthread.h>
#include <inttypes.h>

namespace db
{
namespace rt
{

/**
 * A Monitor provides mutual exclusion and synchronization capabilities for
 * ExclusiveLocks. The actual routines that can be executed inside of a Monitor
 * are provided by the ExclusiveLocks themselves, where each ExclusiveLock has
 * its own Monitor.
 *
 * A Monitor allows a single thread to enter a critical area and execute some
 * routine, prohibiting all other threads from entering the same area at
 * the same time. It also allows for threads to be synchronized with each other
 * by providing an anonymous condition variable that threads can wait on until
 * they are signaled to proceed.
 *
 * Note: This Monitor assumes that no thread will be assigned an invalid ID. If
 * a thread is, then there is a race condition that could result in that
 * thread obtaining a lock when it isn't really inside of this Monitor.
 *
 * db::rt::Thread disallows threads from being created with an invalid ID.
 *
 * @author Dave Longley
 */
class Monitor
{
private:
   /**
    * The mutex for this Monitor.
    */
   pthread_mutex_t mMutex;

   /**
    * The condition used to wait and signal threads.
    */
   pthread_cond_t mWaitCondition;

   /**
    * Stores the pthread ID of the thread that is currently in this Monitor.
    */
   pthread_t mThreadId;

   /**
    * A counter for the number of requested locks by the current Thread.
    */
   uint32_t mLockCount;

public:
   /**
    * Creates a new Monitor.
    */
   Monitor();

   /**
    * Destructs this Monitor.
    */
   ~Monitor();

   /**
    * Causes the current thread to enter this Monitor, thereby prohibiting
    * other threads from entering. This method will acquire a mutual exclusion
    * lock, if the current thread has not already acquired it.
    */
   void enter();

   /**
    * Causes the current thread to exit this Monitor, thereby allowing other
    * threads to enter. This method will release the mutual exclusion lock
    * from the current thread.
    */
   void exit();

   /**
    * Causes the current thread to wait on this Monitor's wait condition until
    * it is signaled or until the specified timeout is reached.
    *
    * @param timeout the number of milliseconds to wait before timing out,
    *                0 to wait indefinitely.
    */
   void wait(uint32_t timeout = 0);

   /**
    * Releases the wait condition and signals a single thread to wake up. The
    * method for choosing the thread that will receive the signal is arbitrary.
    */
   void notify();

   /**
    * Releases the wait condition and signals all threads to wake up.
    */
   void notifyAll();

   /**
    * Signals all threads to wake up without releasing the wait condition. This
    * allows threads to perform maintenance code on themselves before
    * re-entering (or deciding not to) a waiting state.
    */
   void signalAll();
};

} // end namespace rt
} // end namespace db
#endif
