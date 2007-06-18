/*
 * Copyright (c) 2003-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.util.Collection;
import java.util.LinkedList;
import java.util.Random;

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
    * A random number generator for unfair releasing of waiting threads.
    */
   protected Random mRandom;

   /**
    * The threads that may be waiting to acquire a permit.
    */
   protected LinkedList<Thread> mWaitingThreads;
   
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
      
      // create random
      mRandom = new Random();

      // create threads vector
      mWaitingThreads = new LinkedList<Thread>();
      
      // create lock object
      mLockObject = new Object();
   }

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
   protected synchronized int increasePermitsLeft(int increase)
   {
      // only increase, at most, by the difference between the
      // max permit count and the permits left
      increase = Math.min(mPermits - mPermitsLeft, increase);
      
      mPermitsLeft += increase;
      
      return increase;
   }
   
   /**
    * Decreases the number of permits left by the specified number.
    *
    * @param decrease the number of permits to decrease by.
    */
   protected synchronized void decreasePermitsLeft(int decrease)
   {
      mPermitsLeft -= decrease;
   }
   
   /**
    * Tells the current thread to wait.
    * 
    * @throws InterruptedException
    */
   protected void waitThread() throws InterruptedException
   {
      // add thread to waiting threads
      addWaitingThread(Thread.currentThread());
      
      try
      {
         // synchronize on lock object
         synchronized(mLockObject)
         {
            // wait while in the list of waiting threads
            while(mustWait(Thread.currentThread()))
            {
               mLockObject.wait();
            }
         }
      }
      catch(InterruptedException e)
      {
         // maintain interrupted status
         Thread.currentThread().interrupt();
         
         // notify threads
         notifyThreads();
         
         // remove waiting thread
         removeWaitingThread(Thread.currentThread());

         // throw exception
         throw e;
      }
   }
   
   /**
    * Notifies thread(s) to wake up.
    */
   protected synchronized void notifyThreads()
   {
      if(isFair())
      {
         // remove the first waiting thread
         removeFirstWaitingThread();
      }
      else
      {
         // remove a random waiting thread
         removeRandomWaitingThread();
      }

      // synchronize on lock object
      synchronized(mLockObject)
      {
         // notify all threads to wake up
         mLockObject.notifyAll();
      }
   }
   
   /**
    * Adds the passed thread to the queue of waiting threads if it
    * is not already in the queue.
    * 
    * @param thread the thread to add to the queue.
    */
   protected synchronized void addWaitingThread(Thread thread)
   {
      // if this thread isn't in the wait queue, add it
      if(!mWaitingThreads.contains(thread))
      {
         mWaitingThreads.add(thread);
      }
   }
   
   /**
    * Removes the passed thread from the queue of waiting threads. 
    * 
    * @param thread the thread to remove from the wait queue. 
    */
   protected synchronized void removeWaitingThread(Thread thread)
   {
      // remove thread
      mWaitingThreads.remove(thread);
   }
   
   /**
    * Removes the first waiting thread from the queue of waiting threads,
    * if at least one thread is waiting. 
    */
   protected synchronized void removeFirstWaitingThread()
   {
      if(mWaitingThreads.size() > 0)
      {
         // remove first thread
         mWaitingThreads.poll();
      }
   }

   /**
    * Removes a pseudo-random waiting thread from the queue of waiting threads,
    * if at least one thread is waiting.
    */
   protected synchronized void removeRandomWaitingThread()
   {
      if(mWaitingThreads.size() > 0)
      {
         // get a random index
         int index = mRandom.nextInt(mWaitingThreads.size());
         
         // remove the thread
         mWaitingThreads.remove(index);
      }
   }
   
   /**
    * Returns true if the passed thread is in the list of waiting threads,
    * and therefore must wait, false if not.
    *
    * @param thread the thread to check.
    * 
    * @return true if the thread is in the list of waiting threads, and
    *         therefore must wait, false if not.
    */
   protected synchronized boolean mustWait(Thread thread)
   {
      return mWaitingThreads.contains(thread);
   }

   /**
    * Acquires a permit, or blocks until one is
    * available or until interrupted.
    *
    * @throws InterruptedException
    */
   public void acquire() throws InterruptedException
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
   public void acquire(int permits) throws InterruptedException
   {
      // while there are not enough permits, wait for them
      while(availablePermits() - permits < 0)
      {
         // wait thread
         waitThread();
      }
      
      // permits have been granted, decrease permits left
      decreasePermitsLeft(permits);
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
         // decrease permits left
         decreasePermitsLeft(permits);
         
         // permits granted
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
   public synchronized Collection<Thread> getQueuedThreads()
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
      
      // release more permits or decrease permits left
      if(max > oldPermitCount)
      {
         // release used permits
         release(max - oldPermitCount);
      }
      else
      {
         // decrease permits left
         decreasePermitsLeft(oldPermitCount - max);
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
