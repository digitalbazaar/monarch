/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_Monitor_H
#define db_rt_Monitor_H

#include <pthread.h>

namespace db
{
namespace rt
{

/**
 * A Monitor provides mutual exclusion and synchronization capabilities for
 * Objects. The actual routines that can be executed inside of a Monitor are
 * provided by the Objects themselves, where each Object has its own Monitor.
 * 
 * A Monitor allows a single thread to enter a critical area and execute some
 * routine, prohibiting all other threads from entering the same area at
 * the same time. It also allows for threads to be synchronized with each other
 * by providing an anonymous condition variable that threads can wait on until
 * they are signaled to proceed.
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
    * The mutex attributes for this Monitor.
    */
   pthread_mutexattr_t mMutexAttributes;
   
   /**
    * The condition used to wait and signal threads.
    */
   pthread_cond_t mWaitCondition;
   
   /**
    * A counter for the number of requested locks by the current Thread.
    */
   unsigned int mLockCount;
   
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
   void wait(unsigned long timeout = 0);
   
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
