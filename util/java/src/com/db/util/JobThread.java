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
    * Creates a new JobThread. 
    */
   public JobThread()
   {
      // no Runnable job to run yet
      mJob = null;
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
      getLogger().detail("JobThread: going idle.");
      
      try
      {
         // wait
         wait();
      }
      catch(InterruptedException e)
      {
         // ensure interrupted flag remains flipped
         interrupt();
         getLogger().detail("JobThread: interrupted.");
      }
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
      getLogger().detail("JobThread: started.");
      
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
                  // FIXME: only print stack trace at debug verbosity
                  getLogger().error(
                     "JobThread: An exception occurred " +
                     "while running a job,\ntrace= " +
                     LoggerManager.getStackTrace(t));
                  //getLogger().debug(Logger.getStackTrace(t));
               }
               
               // thread no longer has job
               setJob(null);
            }
            
            // go idle
            goIdle();
         }
      }
      catch(Throwable t)
      {
         // FIXME: only print stack trace at debug verbosity
         getLogger().error(
            "JobThread: An exception occurred on a JobThread,\ntrace= " +
            LoggerManager.getStackTrace(t));
         //getLogger().debug(Logger.getStackTrace(t));
      }
      
      getLogger().detail("JobThread: terminated.");
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
    * Gets the logger for this thread.
    * 
    * @return the logger for this thread.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbutil");
   }
}
