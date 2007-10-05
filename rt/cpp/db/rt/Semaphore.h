/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_Semaphore_H
#define db_rt_Semaphore_H

#include "db/rt/Object.h"
#include <list>

namespace db
{
namespace rt
{

// forward declare Thread
class Thread;

/**
 * A Semaphore class that stores the maximum number of permits allowed
 * to be issued -- and allows that number to be dynamically modified.
 * 
 * @author Dave Longley
 */
class Semaphore : public virtual Object
{
protected:
   /**
    * The number of permits.
    */
   int mPermits;

   /**
    * The number of permits left.
    */
   int mPermitsLeft;
   
   /**
    * True if this semaphore guarantees FIFO, false if not.
    */
   bool mFair;
   
   /**
    * The threads that may be waiting to acquire a permit.
    */
   std::list<Thread*> mWaitingThreads;
   
   /**
    * Increases the number of permits left by the specified number if
    * an increase by that number would not exceed the maximum permit count.
    * Otherwise the number of permits left will be set to the maximum
    * permit count.
    *
    * @param increase the number of permits to increase by.
    * 
    * @return the actual increase in the number of permits left.
    */
   int increasePermitsLeft(int increase);
   
   /**
    * Decreases the number of permits left by the specified number.
    *
    * @param decrease the number of permits to decrease by.
    */
   void decreasePermitsLeft(int decrease);
   
   /**
    * Tells the current thread to wait.
    * 
    * @param t the current thread.
    * 
    * @return an InterruptedException if the thread is interrupted, NULL if not.
    */
   InterruptedException* waitThread(Thread* t);
   
   /**
    * Notifies thread(s) to wake up.
    */
   void notifyThreads();
   
   /**
    * Adds the passed thread to the queue of waiting threads if it
    * is not already in the queue.
    * 
    * @param thread the thread to add to the queue.
    */
   void addWaitingThread(Thread* thread);
   
   /**
    * Removes the passed thread from the queue of waiting threads. 
    * 
    * @param thread the thread to remove from the wait queue. 
    */
   void removeWaitingThread(Thread* thread);
   
   /**
    * Removes the first waiting thread from the queue of waiting threads,
    * if at least one thread is waiting. 
    */
   void removeFirstWaitingThread();
   
   /**
    * Removes a pseudo-random waiting thread from the queue of waiting threads,
    * if at least one thread is waiting.
    */
   void removeRandomWaitingThread();
   
   /**
    * Returns true if the passed thread is in the list of waiting threads,
    * and therefore must wait, false if not.
    *
    * @param thread the thread to check.
    * 
    * @return true if the thread is in the list of waiting threads, and
    *         therefore must wait, false if not.
    */
   bool mustWait(Thread* thread);

public:
   /**
    * Constructs a new semaphore with the specified number of
    * permits and fairness setting.
    *
    * @param permits the number of permits.
    * @param fair true if this semaphore guarantees FIFO, false if not.
    */
   Semaphore(int permits, bool fair);
   
   /**
    * Destructs this Semaphore.
    */
   virtual ~Semaphore();
   
   /**
    * Acquires a permit, or blocks until one is available or until interrupted.
    *
    * @return an InterruptedException if the thread is interrupted, NULL if not.
    */
   InterruptedException* acquire();
   
   /**
    * Acquires the given number of permits or blocks until one
    * is available or until interrupted.
    * 
    * @param permits the number of permits to acquire.
    * 
    * @return an InterruptedException if the thread is interrupted, NULL if not.
    */
   InterruptedException* acquire(int permits);
   
   /**
    * Acquires a permit if one is available.
    *
    * @return true if a permit was granted, false if not.
    */
   bool tryAcquire();
   
   /**
    * Acquires the number of given permits if they are available.
    *
    * @param permits the number of permits to acquire.
    * 
    * @return true if the permits were granted, false if not.
    */
   bool tryAcquire(int permits);
   
   /**
    * Releases a permit if it does not exceed the number
    * of used permits.
    */
   void release();
   
   /**
    * Releases the specified number of permits if it does not exceed the
    * number of used permits. If it does, then the number of used
    * permits will be released.
    *
    * @param permits the number of permits to release.
    * 
    * @return the actual number of permits released.
    */
   int release(int permits);
   
   /**
    * Returns the number of available permits.
    *
    * @return the number of available permits.
    */
   int availablePermits();
   
   /**
    * Returns the number of used permits.
    *
    * @return the number of used permits.
    */
   int usedPermits();
   
   /**
    * Returns true if this semaphore guarantees FIFO, false
    * if not.
    *
    * @return true if this semaphore guarantees FIFO, false if not.
    */
   bool isFair();
   
   /**
    * Returns a collection containing threads that may be waiting
    * to acquire permits.
    *
    * @return collection of threads that may be waiting for permits.
    */
   const std::list<Thread*>& getQueuedThreads();
   
   /**
    * Returns an estimate of the number of threads that are
    * waiting to acquire a permit.
    * 
    * @return the number of estimated waiting threads.
    */
   int getQueueLength();
      
   /**
    * Sets the maximum number of permits.
    * 
    * @param max the maximum number of permits.
    */
   void setMaxPermitCount(int max);
      
   /**
    * Gets the maximum number of permits.
    * 
    * @return the maximum number of permits.
    */
   int getMaxPermitCount();
};

} // end namespace rt
} // end namespace db
#endif
