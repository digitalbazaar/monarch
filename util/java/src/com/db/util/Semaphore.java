/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.util.Collection;
import java.util.Vector;

/**
 * A basic Semaphore class. The implemented API matches that of
 * Java's 1.5.0 version of a Semaphore so moving bitmunk to
 * Java 1.5.0+ will be easy.
 * 
 * @author Dave Longley
 */
public class Semaphore
{
   /**
    * The number of permits.
    */
   private int mPermits;

   /**
    * The number of permits left.
    */
   private int mPermitsLeft;

   /**
    * True if this semaphore guarantees FIFO, false if not.
    */
   private boolean mFair;

   /**
    * The threads that may be waiting to acquire a permit.
    */
   private Vector mWaitingThreads;

   /**
    * Constructs a new semaphore with the specified number of
    * permits and fairness setting.
    *
    * @param permits the number of permits.
    * @param fair true if this semaphore guarantees FIFO, false if not.
    */
   public Semaphore(int permits, boolean fair)
   {
      mPermits = permits;
      mPermitsLeft = permits;
      mFair = fair;

      mWaitingThreads = new Vector();
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
    * Increases the number of permits by the specified number.
    *
    * @param increase the number of permits to increase by.
    */
   protected synchronized void increasePermitsLeft(int increase)
   {
      mPermits = availablePermits() + usedPermits() + increase;
      
      mPermitsLeft += increase;
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
            mWaitingThreads.add(Thread.currentThread());
         
         try
         {
            wait();
         }
         catch(InterruptedException e)
         {
            if(isFair())
            {
               notify();
            }
            else
            {
               notifyAll();
            }
            
            // if the thread is in the wait queue, remove it
            mWaitingThreads.remove(Thread.currentThread());

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
    * Releases the specified number of permits.
    *
    * @param permits the number of permits to release.
    */
   public synchronized void release(int permits)
   {
      // increase the number of permits left
      increasePermitsLeft(permits);

      // if fair, notify in order, otherwise notify all
      for(int i = 0; i < permits; i++)
      {
         if(isFair())
         {
            notify();
         }
         else
         {
            notifyAll();
         }
      }
   }

   /**
    * Returns the number of available permits.
    *
    * @return the number of available permits.
    */
   public int availablePermits()
   {
      return mPermitsLeft;
   }
   
   /**
    * Returns the number of used permits.
    *
    * @return the number of used permits.
    */
   public int usedPermits()
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
   public Collection getQueuedThreads()
   {
      return mWaitingThreads;
   }

   /**
    * Returns an estimate of the number of threads that are
    * waiting to acquire a permit.
    * 
    * @return the number of estimated waiting threads.
    */
   public int getQueueLength()
   {
      return mWaitingThreads.size();
   }
   
   /**
    * Sets the maximum number of permits.
    * 
    * @param max the maximum number of permits.
    */
   public void setNumPermits(int max)
   {
      if(max > mPermits)
      {
         release(max - mPermits);
      }
      else
      {
         reducePermitsLeft(mPermits - max);
      }
      
      mPermits = max;
   }
   
   /**
    * gets the maximum number of permits.
    * 
    * @return the maximum number of permits.
    */
   public int getNumPermits()
   {
      return availablePermits() + usedPermits();
   }
}
