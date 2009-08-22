/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.util.Iterator;
import java.util.LinkedList;

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
   protected LinkedList<Runnable> mJobQueue;   
   
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
      mJobQueue = new LinkedList<Runnable>();
      
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
      
      if(job != null)
      {
         // add the job to the queue
         rval = mJobQueue.add(job);
      }
      
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
         rval = mJobQueue.poll();
      }
      
      return rval;
   }
   
   /**
    * Gets an iterator over the jobs in the queue (in FIFO order).
    *
    * @return an iterator over the jobs in the queue.
    */
   protected Iterator<Runnable> getJobIterator()
   {
      return mJobQueue.iterator();
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
    * Queues a Runnable job for execution.
    * 
    * @param job the Runnable job to queue.
    */
   public synchronized void queueJob(Runnable job)
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
    * Dispatches the next Runnable job in the queue, if one exists.
    */
   public void dispatchNextJob()
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
      if(!isDispatching())
      {
         getLogger().debug(getClass(),
            "JobDispatcher starting dispatching...");

         // create new dispatcher thread
         mDispatcherThread = new Thread(this);
         
         // start dispatcher thread
         mDispatcherThread.start();
         
         getLogger().debug(getClass(),
            "JobDispatcher started dispatching.");
      }
      else
      {
         getLogger().debug(getClass(),
            "JobDispatcher is already dispatching.");
      }
   }
   
   /**
    * Stops dispatching Runnable jobs. This does not terminate the jobs
    * that are already running.
    */
   public synchronized void stopDispatching()
   {
      if(isDispatching())
      {
         getLogger().debug(getClass(),
            "JobDispatcher stopping dispatching...");
         
         // interrupt dispatcher thread
         getDispatcherThread().interrupt();
         
         // clean up dispatcher thread
         mDispatcherThread = null;
         
         getLogger().debug(getClass(),
            "JobDispatcher stopped dispatching.");
      }
      else
      {
         getLogger().debug(getClass(),
            "JobDispatcher is already not dispatching.");
      }
   }
   
   /**
    * Returns true if this JobDispatcher is dispatching jobs, false if not.
    * 
    * @return true if this JobDispatcher is dispatching jobs, false if not.
    */
   public synchronized boolean isDispatching()
   {
      return (getDispatcherThread() != null);
   }
   
   /**
    * Removes all queued jobs. Already running jobs will not be affected.
    */
   public synchronized void clearQueuedJobs()
   {
      mJobQueue.clear();
   }
   
   /**
    * Interrupts all running Jobs. Queued jobs will not be affected.
    */
   public synchronized void interruptAllRunningJobs()
   {
      getThreadPool().interruptAllThreads();
   }
   
   /**
    * Terminates all running Jobs. Queued jobs will not be affected. This
    * method will wait until all of the threads are joined.
    */
   public synchronized void terminateAllRunningJobs()
   {
      getThreadPool().terminateAllThreads();
   }   
   
   /**
    * Terminates all running Jobs. Queued jobs will not be affected.
    * 
    * @param joinTime the amount of time (in milliseconds) to wait to join
    *                 the running threads.
    */
   public synchronized void terminateAllRunningJobs(long joinTime)
   {
      getThreadPool().terminateAllThreads(joinTime);
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
