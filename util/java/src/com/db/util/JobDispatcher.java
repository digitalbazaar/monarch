/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A JobDispatcher is a class that maintains a queue of Runnable jobs
 * that are dispatched on a separate thread to a JobThreadPool.
 * 
 * @author Dave Longley
 */
public class JobDispatcher implements Runnable
{
   /**
    * A thread pool for running jobs. This thread pool maintains
    * a set of threads that are available for running jobs.
    */
   protected JobThreadPool mThreadPool;
   
   /**
    * The internal queue that holds the Runnable jobs that are
    * waiting to be dispatched.
    */
   protected Vector mJobQueue;   
   
   /**
    * The thread used to dispatch the Runnable jobs.
    */
   protected Thread mDispatcherThread;
   
   /**
    * Creates a new JobDispatcher.
    */
   public JobDispatcher()
   {
      // create the job thread pool with an infinite number of
      // threads by default
      this(new JobThreadPool(0));
      
      // set thread expire time to 5 minutes (300000 milliseconds) by default
      getThreadPool().setJobThreadExpireTime(300000);
   }
   
   /**
    * Creates a new JobDispatcher with the given JobThreadPool.
    * 
    * @param pool the JobThreadPool to dispatch jobs to. 
    */
   public JobDispatcher(JobThreadPool pool)
   {
      // store the thread pool
      mThreadPool = pool;
      
      // create the job queue
      mJobQueue = new Vector();
      
      // no dispatcher thread yet
      mDispatcherThread = null;
   }
   
   /**
    * Adds a Runnable job to the queue.
    * 
    * @param job the Runnable job to add to the queue.
    * 
    * @return true if added or already in the queue, false if not.
    */
   protected synchronized boolean pushJob(Runnable job)
   {
      boolean rval = false;
      
      // add the job to the queue
      rval = mJobQueue.add(job);
      
      return rval;
   }
   
   /**
    * Pops the next Runnable job off of the queue.
    * 
    * @return job the popped Runnable job.
    */
   protected synchronized Runnable popJob() 
   {
      Runnable rval = null;
      
      if(!mJobQueue.isEmpty())
      {
         // remove the top Runnable job
         rval = (Runnable)mJobQueue.remove(0);
      }
      
      return rval;
   }
   
   /**
    * Gets the dispatcher thread.
    * 
    * @return the dispatcher thread.
    */
   protected Thread getDispatcherThread()
   {
      return mDispatcherThread;
   }
   
   /**
    * Dispatches the next Runnable job in the queue, if one exists.
    */
   protected void dispatchNextJob()
   {
      // pop next Runnable job off of the queue
      Runnable job = popJob();
      if(job != null)
      {
         // run the job
         getThreadPool().runJob(job);      
      }
   }
   
   /**
    * Queues a Runnable job for execution.
    * 
    * @param job the Runnable job to queue.
    */
   public void queueJob(Runnable job)
   {
      pushJob(job);
   }
   
   /**
    * Dequeues a Runnable job so that it will no longer be
    * executed if it has not already begun execution.
    * 
    * @param job the Runnable job to dequeue.
    */
   public synchronized void dequeueJob(Runnable job)
   {
      mJobQueue.remove(job);
   }
   
   /**
    * Returns true if the passed Runnable job is in the queue
    * to be dispatched, false if not.
    *
    * @param job the Runnable job to look for.
    *
    * @return true if the passed Runnable job is in the queue to
    *         be dispatched, false if not.
    */
   public synchronized boolean isQueued(Runnable job)
   {
      return mJobQueue.contains(job);
   }
   
   /**
    * Starts dispatching Runnable jobs.
    */
   public synchronized void startDispatching()
   {
      if(getDispatcherThread() == null)
      {
         getLogger().debug(getClass(), "JobDispatcher started.");
         
         // create new dispatcher thread
         mDispatcherThread = new Thread(this);
         
         // start dispatcher thread
         mDispatcherThread.start();
      }
      else
      {
         getLogger().debug(getClass(), "JobDispatcher already started.");
      }
   }
   
   /**
    * Stops dispatching Runnable jobs. This does not terminate the jobs
    * that are already running.
    */
   public synchronized void stopDispatching()
   {
      if(getDispatcherThread() != null)
      {
         getLogger().debug(getClass(), "Stopping JobDispatcher...");
         
         // interrupt dispatcher thread
         getDispatcherThread().interrupt();
         
         getLogger().debug(getClass(), "JobDispatcher stopped.");
      }
      else
      {
         getLogger().debug(getClass(), "JobDispatcher already stopped.");
      }
   }
   
   /**
    * Removes all queued jobs. Already running jobs will not be affected.
    */
   public synchronized void clearQueuedJobs()
   {
      mJobQueue.clear();
   }
   
   /**
    * Terminates all running Jobs. Queued jobs will not be affected.
    */
   public synchronized void terminateAllRunningJobs()
   {
      getThreadPool().terminateAllThreads();
   }
   
   /**
    * Called to start dispatching Runnable jobs.
    */
   public void run()
   {
      try
      {
         while(!Thread.currentThread().isInterrupted())
         {
            // dispatch the next Runnable job
            dispatchNextJob();
            
            // sleep
            Thread.sleep(1);
         }
      }
      catch(InterruptedException e)
      {
         // toggle interrupted flag to true
         Thread.currentThread().interrupt();
      }
   }
   
   /**
    * Gets the JobThreadPool.
    * 
    * @return the thread pool for running jobs.
    */
   public JobThreadPool getThreadPool()
   {
      return mThreadPool;
   }
   
   /**
    * Gets the number of Runnable jobs that are in the queue.
    * 
    * @return the number of Runnable jobs that are queued to be dispatched.
    */
   public synchronized int getQueuedJobCount()
   {
      return mJobQueue.size();
   }
   
   /**
    * Gets the number of Runnable jobs that are in the queue and that are
    * running.
    * 
    * @return the number of Runnable jobs that are queued to be dispatched
    *         plus the Runnable jobs that are already running.
    */
   public synchronized int getTotalJobCount()
   {
      return getQueuedJobCount() + getThreadPool().getIdleJobThreadCount();
   }
   
   /**
    * Gets the logger for this dispatcher.
    * 
    * @return the logger for this dispatcher.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbutil");
   }
}
