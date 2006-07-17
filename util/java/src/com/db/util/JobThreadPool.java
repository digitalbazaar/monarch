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
    * The expire time for JobThreads.
    */
   protected long mJobThreadExpireTime;
   
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
      
      // default JobThread expire time to 0 (no expiration)
      setJobThreadExpireTime(0);
      
      // set the pool size (number of threads)
      setPoolSize(poolSize);
   }
   
   /**
    * When finalizing this thread pool, terminate all of its threads.
    */
   public void finalize()
   {
      // terminate all threads
      terminateAllThreads();
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
      // If this pool allows an infinite number of threads, then
      // the number of permits will be zero -- since threads are
      // always permitted. Therefore, only try to acquire a
      // permit if there are more than 0 permits -- otherwise a
      // permit is automatically granted.
      if(mThreadSemaphore.getMaxPermitCount() != 0)
      {
         mThreadSemaphore.acquire();
      }
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
      // create job thrad
      JobThread thread = new JobThread(getJobThreadExpireTime());
      return thread;
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
      int extraThreads = 0;
      if(mThreadSemaphore.getMaxPermitCount() > 0)
      {
         extraThreads = mThreads.size() - mThreadSemaphore.getMaxPermitCount();
      }
      
      // iterate through threads, find one that is idle
      for(Iterator i = mThreads.iterator(); i.hasNext();)
      {
         JobThread thread = (JobThread)i.next();
         if(thread.isIdle())
         {
            // if the thread is not alive, remove it and continue on
            if(!thread.isAlive())
            {
               getLogger().detail(getClass(), "removing expired thread.");
               
               // interrupt thread just in case
               thread.interrupt();
               
               // remove thread
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
                  getLogger().detail(getClass(), "removing extra thread.");
                  
                  // interrupt thread
                  thread.interrupt();
                  
                  // remove it from the pool
                  i.remove();
                  
                  // decrement extra threads
                  extraThreads--;
               }
               else if(rval == null)
               {
                  // return this thread
                  rval = thread;
               }
            }
         }
      }
      
      // if no idle thread was found
      if(rval == null)
      {
         // create new job thread
         rval = createJobThread();
         
         getLogger().detail(getClass(), "adding new thread.");
         
         // add thread to pool
         mThreads.add(rval);
         
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
    * Sets the number of threads in this thread pool. If a size of
    * 0 is specified, than there will be no limit to the number of
    * threads in this pool.
    * 
    * @param size the number of threads in this thread pool. A size
    *             of 0 specifies an unlimited number of threads. Size
    *             must be a non-negative number.
    */
   public synchronized void setPoolSize(int size)
   {
      // disallow any size that is negative
      if(size < 0)
      {
         throw new IllegalArgumentException("Thread pool size must be >= 0");
      }
      
      // Note: threads are created lazily so if the thread pool size
      // here is greater than the number of threads currently in
      // the pool, those threads will be created as they are needed
      // hence, we do not need to adjust for increases in the pool size
      // only for decreases
      
      // remove threads as necessary
      if(mThreads.size() > size && size != 0)
      {
         // get number of threads to remove
         int removeCount = mThreads.size() - size;
         
         // iterate through threads, remove idle threads
         for(Iterator i = mThreads.iterator(); i.hasNext() && removeCount > 0;)
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
      mThreadSemaphore.setMaxPermitCount(size);
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
         
         getLogger().debug(getClass(), 
            "thread acquisition interrupted.");
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
      getLogger().detail(getClass(), "terminating all threads.");
      
      // iterate through all threads, interrupt and remove them
      for(Iterator i = mThreads.iterator(); i.hasNext();)
      {
         JobThread thread = (JobThread)i.next();
         
         // interrupt thread
         thread.interrupt();
         
         // remove thread
         i.remove();
      }
      
      getLogger().detail(getClass(), "all threads terminated.");      
   }
   
   /**
    * Sets the expire time for all JobThreads.
    * 
    * @param expireTime the amount of time that must pass while JobThreads
    *                   are idle in order for them to expire -- if 0 is passed
    *                   then JobThreads will never expire.
    */
   public synchronized void setJobThreadExpireTime(long expireTime)
   {
      // expire time must be non-negative
      if(expireTime < 0)
      {
         throw new IllegalArgumentException(
            "JobThread expire time must be >= 0");
      }
      
      mJobThreadExpireTime = expireTime;
      
      // update all existing job threads
      Iterator i = mThreads.iterator();
      while(i.hasNext())
      {
         JobThread thread = (JobThread)i.next();
         thread.setExpireTime(expireTime);
      }
   }
   
   /**
    * Gets the expire time for all JobThreads.
    * 
    * @return the expire time for all JobThreads.
    */
   public synchronized long getJobThreadExpireTime()
   {
      return mJobThreadExpireTime;
   }
   
   /**
    * Gets the current number of JobThreads in the pool.
    * 
    * @return the current number of JobThreads in the pool.
    */
   public synchronized int getJobThreadCount()
   {
      return mThreads.size();
   }
   
   /**
    * Gets the current number of running JobThreads.
    * 
    * Returns getJobThreadCount() - getIdleJobThreadCount().
    * 
    * @return the current number of running JobThreads.
    */
   public synchronized int getRunningJobThreadCount()
   {
      int rval = 0;
      
      // subtract idle threads from total threads
      rval = getJobThreadCount() - getIdleJobThreadCount();
      
      return rval;
   }
   
   /**
    * Gets the current number of idle JobThreads.
    * 
    * @return the current number of idle JobThreads.
    */
   public synchronized int getIdleJobThreadCount()
   {
      int rval = 0;
      
      // iterate through all threads, add up idle threads
      for(Iterator i = mThreads.iterator(); i.hasNext();)
      {
         JobThread thread = (JobThread)i.next();
         if(thread.isIdle())
         {
            rval++;
         }
      }
      
      return rval;
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
