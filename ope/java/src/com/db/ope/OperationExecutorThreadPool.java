/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

import com.db.util.JobThreadPool;

/**
 * An OperationExecutorThreadPool is a JobThreadPool whose JobThreads run
 * OperationExecutors.
 * 
 * @author Dave Longley
 */
public class OperationExecutorThreadPool extends JobThreadPool
{
   /**
    * Creates a new OperationExecutorThreadPool.
    */
   public OperationExecutorThreadPool()
   {
      // create the operation executor thread pool with an infinite number of
      // threads for running OperationExecutors by default
      super(0);
      
      // set expire time to 5 minutes (300000 milliseconds) for
      // threads that execute operations
      setJobThreadExpireTime(300000);
   }
   
   /**
    * Runs the passed Runnable job (An OperationExecutor) on an available
    * JobThread.
    * 
    * This method will lock until an available thread is acquired or
    * the current thread is interrupted.
    * 
    * @param job the Runnable job to run (a ContractProcessor).
    * 
    * @exception ClassCastException thrown if the passed job is not an
    *                               OperationExecutor.
    */
   @Override
   public void runJob(Runnable job)
   {
      // cast job to OperationExecutor
      OperationExecutor executor = (OperationExecutor)job;
      
      // operation execution has begun
      executor.setStarted(true);

      // run operation executor
      super.runJob(executor);
   }
}
