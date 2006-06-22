/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.util.Semaphore;

/**
 * This class is a JobThread pool.
 * 
 * This pool maintains a set of N JobThreads that can be used to run jobs
 * without having to tear down the threads and create new ones.
 * 
 * @author Dave Longley
 */
public class JobThreadPool
{
   /**
    * This semaphore is used to regulate the number of threads that
    * are available in this pool.
    */
   protected Semaphore mThreadSemaphore;
   
   /**
    * The list of threads in this pool. This is the list of total threads
    * in this pool.
    */
   protected Vector mThreads;
   
   /**
    * Creates a new ThreadPool with 10 threads available for running jobs.
    */
   public JobThreadPool()
   {
      this(10);
   }

   /**
    * Creates a new ThreadPool with the specified number of
    * threads available for running jobs.
    * 
    * @param poolSize the size of the pool (number of threads).
    */
   public JobThreadPool(int poolSize)
   {
      // create the thread semaphore
      mThreadSemaphore = new Semaphore(poolSize, true);
      
      // create the thread list
      mThreads = new Vector();
      
      // set the pool size (number of threads)
      setPoolSize(poolSize);
   }
   
   /**
    * Acquires a thread permit for running a job.
    * 
    * This method will lock until an available thread is acquired or
    * the current thread is interrupted.
    * 
    * @throws InterruptedException
    */
   protected void acquireThreadPermit() throws InterruptedException
   {
      mThreadSemaphore.acquire();
   }
   
   /**
    * Releases a thread permit that was used to run a job.
    */
   protected void releaseThreadPermit()
   {
      mThreadSemaphore.release();
   }
   
   /**
    * Creates a new JobThread.
    * 
    * @return the new JobThread.
    */
   protected synchronized JobThread createJobThread()
   {
      return new JobThread();
   }
   
   /**
    * Gets an idle thread. This method will also clean up any extra
    * idle threads that should not exist due to a decrease in the
    * pool size. If an idle thread is not found, it will be created.
    * 
    * A thread permit should always be acquired before calling this
    * method -- as to do otherwise will cause the number of threads to
    * become out of sync with the thread semaphore permit count.
    * 
    * @return an idle thread.
    */
   protected synchronized JobThread getIdleThread()
   {
      JobThread rval = null;
      
      // get the number of extra threads
      int extraThreads = mThreads.size() - mThreadSemaphore.getNumPermits();
      
      // iterate through threads, find one that is idle
      Iterator i = mThreads.iterator();
      while(i.hasNext())
      {
         JobThread thread = (JobThread)i.next();
         if(thread.isIdle())
         {
            // if the thread is not alive, remove it and continue on
            if(thread.isAlive())
            {
               i.remove();
               
               // decrement extra threads
               extraThreads--;
            }
            else
            {
               // if there are extra idle threads, interrupt and remove
               // this one
               if(extraThreads > 0)
               {
                  // interrupt thread
                  thread.interrupt();
                  
                  // remove it from the pool
                  i.remove();
                  
                  // decrement extra threads
                  extraThreads--;
               }
               else
               {
                  // return this thread
                  rval = thread;
                  break;
               }
            }
         }
      }
      
      // if no idle thread was found
      if(rval == null)
      {
         // create new job thread
         rval = createJobThread();
         
         // start the thread
         rval.start();
      }
      
      return rval;
   }
   
   /**
    * Runs the passed Runnable job on an idle JobThread.
    * 
    * @param job the Runnable job to run.
    */
   protected synchronized void runJobOnIdleThread(Runnable job)
   {
      // get an idle thread
      JobThread thread = getIdleThread();
      
      // set job
      thread.setJob(job);
   }
   
   /**
    * Sets the number of threads in this thread pool.
    * 
    * @param size the number of threads in this thread pool. It must be
    *             a positive number.
    */
   public synchronized void setPoolSize(int size)
   {
      // disallow any size that is non-positive
      if(size <= 0)
      {
         throw new IllegalArgumentException("Thread pool size must be >= 0");
      }
      
      // Note: threads are created lazily so if the thread pool size
      // here is greater than the number of threads currently in
      // the pool, those threads will be created as they are needed
      // hence, we do not need to adjust for increases in the pool size
      // only for decreases
      
      // remove threads as necessary
      if(mThreads.size() > size)
      {
         // get number of threads to remove
         int removeCount = mThreads.size() - size;
         
         // iterate through threads, remove idle threads
         Iterator i = mThreads.iterator();
         while(i.hasNext() && removeCount > 0)
         {
            JobThread thread = (JobThread)i.next();
            
            // if thread is idle, interrupt it and remove it
            if(thread.isIdle())
            {
               // interrupt thread
               thread.interrupt();
               
               // remove it from the pool
               i.remove();
               
               // decrement remove count
               removeCount--;
            }
         }
      }
      
      // set semaphore permits
      mThreadSemaphore.setNumPermits(size);
   }
   
   /**
    * Runs the passed Runnable job on an available JobThread.
    * 
    * This method will lock until an available thread is acquired or
    * the current thread is interrupted.
    * 
    * @param job the Runnable job to run.
    */
   public void runJob(Runnable job)
   {
      boolean permitAcquired = false;
      
      try
      {
         // acquire a thread permit
         acquireThreadPermit();
         
         // permit acquired
         permitAcquired = true;
         
         // run the job on an idle thread
         runJobOnIdleThread(job);
      }
      catch(InterruptedException e)
      {
         // toggle interrupt thread to true
         Thread.currentThread().interrupt();
         
         getLogger().debug(
            "JobThreadPool: thread acquisition interrupted.");
      }
      
      // if a permit was acquired, release it
      if(permitAcquired)
      {
         releaseThreadPermit();
      }
   }
   
   /**
    * Interrupts all threads in this pool and removes them.
    */
   public synchronized void terminateAllThreads()
   {
      // iterate through all threads, interrupt and remove them
      Iterator i = mThreads.iterator();
      while(i.hasNext())
      {
         JobThread thread = (JobThread)i.next();
         
         // interrupt thread
         thread.interrupt();
         
         // remove thread
         i.remove();
      }      
   }   
   
   /**
    * Gets the logger for this thread pool.
    * 
    * @return the logger for this thread pool.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbutil");
   }
}
