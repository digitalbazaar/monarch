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
}
