/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A JobThread is a thread that runs Runnable jobs and sleeps
 * while it is not running a job. This thread can be used in
 * conjunction with a JobThreadPool.
 * 
 * @author Dave Longley
 */
public class JobThread extends Thread
{
   /**
    * The Runnable job to run.
    */
   protected Runnable mJob;
   
   /**
    * The amount of idle time that must pass before this JobThread
    * automatically expires.
    */
   protected long mExpireTime;
   
   /**
    * Creates a new JobThread with no expiration time. 
    */
   public JobThread()
   {
      this(0);
   }
   
   /**
    * Creates a new JobThread that expires if it sits idle (never processes
    * a single job) for the passed time interval. If an expire time of 0
    * is passed then this JobThread will never expire.
    * 
    * @param expireTime the amount of time that must pass while the JobThread
    *                   is idle in order for it to expire -- if 0 is passed
    *                   then the JobThread will never expire.
    */
   public JobThread(long expireTime)
   {
      // no Runnable job to run yet
      mJob = null;
      
      // sets the expire time for this thread
      setExpireTime(expireTime);
   }
   
   /**
    * Gets the Runnable job to run, if any.
    * 
    * @return the Runnable job to run, if any.
    */
   protected synchronized Runnable getJob()
   {
      return mJob;
   }
   
   /**
    * Makes this thread idle.
    */
   protected synchronized void goIdle()
   {
      getLogger().detail(getClass(), "going idle.");
      
      try
      {
         long startTime = System.currentTimeMillis();
         
         // wait until expire time
         wait(getExpireTime());
         
         // if this thread has an expire time set and this thread still has
         // no job see if the time has expired
         if(getExpireTime() != 0 && !hasJob())
         {
            // check expired time
            long now = System.currentTimeMillis();
            if(now - startTime >= getExpireTime())
            {
               getLogger().detail(getClass(), 
                  "idle expire time reached, interrupting...");
               
               // thread must expire
               interrupt();
            }
         }
      }
      catch(InterruptedException e)
      {
         // ensure interrupted flag remains flipped
         interrupt();
         getLogger().detail(getClass(), "interrupted.");
      }
      
      getLogger().detail(getClass(), "no longer idle.");
   }
   
   /**
    * Wakes up this thread.
    */
   protected synchronized void wakeup()
   {
      // notify thread to stop waiting
      notify();
   }
   
   /**
    * Sets the Runnable job for this thread. If null is passed then this
    * thread will be considered idle (with no job).
    * 
    * @param job the job for this thread or null if this thread has no job.
    */
   public synchronized void setJob(Runnable job)
   {
      // set job
      mJob = job;
      
      if(job == null)
      {
         // set thread name
         setName("JobThread: idle");
      }
      else
      {
         // set thread name
         setName("JobThread: running job '" + getJob() + "'");
         
         // wake up thread
         wakeup();
      }
   }
   
   /**
    * Runs this thread.
    */
   public void run()
   {
      getLogger().detail(getClass(), "started.");
      
      try
      {
         while(!isInterrupted())
         {
            // get the Runnable job to run
            Runnable job = getJob();
            if(job != null)
            {
               try
               {
                  // run job
                  job.run();
               }
               catch(Throwable t)
               {
                  getLogger().error(getClass(), 
                     "An exception occurred while running a job" +
                     ",\nexception= " + t);
                  getLogger().debug(getClass(), 
                     "An exception occurred while running a job" +
                     ",\nexception= " + t +
                     ",\ntrace= " + Logger.getStackTrace(t));                  
               }
               
               // thread no longer has job
               setJob(null);
            }
            
            if(!isInterrupted())
            {
               // go idle
               goIdle();
            }
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), 
            "An exception occurred on a JobThread" +
            ",\nexception= " + t);
         getLogger().debug(getClass(), 
            "An exception occurred on a JobThread" +
            ",\nexception= " + t +
            ",\ntrace= " + Logger.getStackTrace(t));                  
      }
      
      getLogger().detail(getClass(), "terminated.");
   }
   
   /**
    * Returns true if this thread has a job, false if it is idle.
    * 
    * @return true if this thread has a job, false if it is idle.
    */
   public synchronized boolean hasJob()
   {
      return getJob() != null;
   }
   
   /**
    * Returns true if this thread has no job, false if it does.
    * 
    * @return true if this thread has no job, false if it does.
    */
   public synchronized boolean isIdle()
   {
      return !hasJob();
   }
   
   /**
    * Sets the expire time for this job thread.
    * 
    * @param expireTime the amount of time that must pass while this JobThread
    *                   is idle in order for it to expire -- if 0 is passed
    *                   then this JobThread will never expire.
    */
   public synchronized void setExpireTime(long expireTime)
   {
      // expire time must be non-negative
      if(expireTime < 0)
      {
         throw new IllegalArgumentException(
            "JobThread expire time must be >= 0");
      }
      
      mExpireTime = expireTime;
   }
   
   /**
    * Gets the expire time for this job thread.
    * 
    * @return the expire time for this job thread.
    */
   public synchronized long getExpireTime()
   {
      return mExpireTime;
   }
   
   /**
    * Gets the logger for this thread.
    * 
    * @return the logger for this thread.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbutil");
   }
}
