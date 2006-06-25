/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.util.Collection;
import java.util.Vector;

/**
 * A Semaphore class that stores the maximum number of permits allowed
 * to be issued -- and allows that number to be dynamically modified.
 * 
 * @author Dave Longley
 */
public class Semaphore
{
   /**
    * The number of permits.
    */
   protected int mPermits;

   /**
    * The number of permits left.
    */
   protected int mPermitsLeft;

   /**
    * True if this semaphore guarantees FIFO, false if not.
    */
   protected boolean mFair;

   /**
    * The threads that may be waiting to acquire a permit.
    */
   protected Vector mWaitingThreads;
   
   /**
    * This condition is set to true when threads must wait,
    * and false when threads are being notified to wake up.
    */
   protected boolean mMustWait;
   
   /**
    * The lock object. This is the object to synchronize on when
    * calling wait() or notify(). 
    */
   protected Object mLockObject;

   /**
    * Constructs a new semaphore with the specified number of
    * permits and fairness setting.
    *
    * @param permits the number of permits.
    * @param fair true if this semaphore guarantees FIFO, false if not.
    */
   public Semaphore(int permits, boolean fair)
   {
      // set permits
      mPermits = permits;
      mPermitsLeft = permits;
      
      // set fair/not fair
      mFair = fair;

      // create threads vector
      mWaitingThreads = new Vector();
      
      // set must wait to false
      mMustWait = false;
      
      // create lock object
      mLockObject = new Object();
   }

   /**
    * Reduces the number of permits by the specified number.
    *
    * @param reduction the number of permits to reduce by.
    */
   protected synchronized void reducePermitsLeft(int reduction)
   {
      mPermitsLeft -= reduction;
   }

   /**
    * Increases the number of permits left by the specified number if
    * that number does not exceed the number of used permits. Otherwise
    * the number of permits left will be increased by the number of
    * used permits.
    *
    * @param increase the number of permits to increase by.
    * 
    * @return the actual increase in the number of permits left.
    */
   protected synchronized int increasePermitsLeft(int increase)
   {
      // only increase, at most, by the number of used permits
      increase = Math.min(usedPermits(), increase);
      
      mPermitsLeft += increase;
      
      return increase;
   }
   
   /**
    * Tells the current thread to wait.
    * 
    * @throws InterruptedException
    */
   protected synchronized void waitThread() throws InterruptedException
   {
      // thread must wait
      mMustWait = true;
      
      // synchronize on lock object
      synchronized(mLockObject)
      {
         while(mMustWait)
         {
            wait();
         }
      }
   }
   
   /**
    * Notifies thread(s) to wake up.
    */
   protected synchronized void notifyThreads()
   {
      // thread(s) are being notified
      mMustWait = false;
      
      if(isFair())
      {
         // synchronize on lock object
         synchronized(mLockObject)
         {
            notify();
         }
      }
      else
      {
         // synchronize on lock object
         synchronized(mLockObject)
         {
            notifyAll();
         }
      }
   }

   /**
    * Acquires a permit, or blocks until one is
    * available or until interrupted.
    *
    * @throws InterruptedException
    */
   public synchronized void acquire()
      throws InterruptedException
   {
      acquire(1);
   }

   /**
    * Acquires the given number of permits or blocks until one
    * is available or until interrupted.
    *
    * @param permits the number of permits to acquire.
    * 
    * @throws InterruptedException
    */
   public synchronized void acquire(int permits)
      throws InterruptedException
   {
      // while there are not enough permits, wait for them
      while(availablePermits() - permits < 0)
      {
         // if this thread isn't in the wait queue, add it
         if(!mWaitingThreads.contains(Thread.currentThread()))
         {
            mWaitingThreads.add(Thread.currentThread());
         }
         
         try
         {
            // wait thread
            waitThread();
         }
         catch(InterruptedException e)
         {
            // notify threads
            notifyThreads();
            
            // if the thread is in the wait queue, remove it
            mWaitingThreads.remove(Thread.currentThread());

            // throw exception
            throw e;
         }
      }

      // if the thread is in the wait queue, remove it
      mWaitingThreads.remove(Thread.currentThread());

      // permits have been granted, decrease number available
      reducePermitsLeft(permits);
   }

   /**
    * Acquires a permit if one is available.
    *
    * @return true if a permit was granted, false if not.
    */
   public synchronized boolean tryAcquire()
   {
      return tryAcquire(1);
   }

   /**
    * Acquires the number of given permits if they are available.
    *
    * @param permits the number of permits to acquire.
    * 
    * @return true if the permits were granted, false if not.
    */
   public synchronized boolean tryAcquire(int permits)
   {
      boolean rval = false;
      
      if(availablePermits() - permits >= 0)
      {
         // if the thread is in the wait queue, remove it
         mWaitingThreads.remove(Thread.currentThread());

         // reduce the number of permits
         reducePermitsLeft(permits);
         
         rval = true;
      }

      return rval;
   }
   
   /**
    * Releases a permit.
    */
   public synchronized void release()
   {
      release(1);
   }

   /**
    * Releases the specified number of permits if it does not exceed the
    * number of used permits. If it does, then the number of used
    * permits will be released.
    *
    * @param permits the number of permits to release.
    * 
    * @return the actual number of permits released.
    */
   public synchronized int release(int permits)
   {
      // increase the number of permits left
      permits = increasePermitsLeft(permits);

      // if fair, notify in order, otherwise notify all
      for(int i = 0; i < permits; i++)
      {
         // notify threads
         notifyThreads();
      }
      
      return permits;
   }

   /**
    * Returns the number of available permits.
    *
    * @return the number of available permits.
    */
   public synchronized int availablePermits()
   {
      return mPermitsLeft;
   }
   
   /**
    * Returns the number of used permits.
    *
    * @return the number of used permits.
    */
   public synchronized int usedPermits()
   {
      return mPermits - mPermitsLeft;
   }

   /**
    * Returns true if this semaphore guarantees FIFO, false
    * if not.
    *
    * @return true if this semaphore guarantees FIFO, false if not.
    */
   public boolean isFair()
   {
      return mFair;
   }

   /**
    * Returns a collection containing threads that may be waiting
    * to acquire permits.
    *
    * @return collection of threads that may be waiting for permits.
    */
   public synchronized Collection getQueuedThreads()
   {
      return mWaitingThreads;
   }

   /**
    * Returns an estimate of the number of threads that are
    * waiting to acquire a permit.
    * 
    * @return the number of estimated waiting threads.
    */
   public synchronized int getQueueLength()
   {
      return mWaitingThreads.size();
   }
   
   /**
    * Sets the maximum number of permits.
    * 
    * @param max the maximum number of permits.
    */
   public synchronized void setMaxPermitCount(int max)
   {
      // store old permit count
      int oldPermitCount = getMaxPermitCount();
      
      // set new permit count
      mPermits = max;
      
      // release or reduce permits left
      if(max > oldPermitCount)
      {
         release(max - oldPermitCount);
      }
      else
      {
         reducePermitsLeft(oldPermitCount - max);
      }
   }
   
   /**
    * Gets the maximum number of permits.
    * 
    * @return the maximum number of permits.
    */
   public synchronized int getMaxPermitCount()
   {
      return mPermits;
   }
}
