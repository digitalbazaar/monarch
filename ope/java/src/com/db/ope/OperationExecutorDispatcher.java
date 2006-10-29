/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

import java.util.Iterator;

import com.db.util.JobDispatcher;

/**
 * An OperationExecutorDispatcher dispatches OperationExecutors in a FIFO
 * fashion. Once an OperationExecutor has been dispatched, it begins executing
 * an Operation.
 * 
 * @author Dave Longley
 */
public class OperationExecutorDispatcher extends JobDispatcher
{
   /**
    * The OperationEngine this dispatcher is for.
    */
   protected OperationEngine mEngine;
   
   /**
    * Creates a new OperationExecutorDispatcher for the given OperationEngine.
    * 
    * @param engine the OperationEngine this dispatcher is for.
    */
   public OperationExecutorDispatcher(OperationEngine engine)
   {
      super(new OperationExecutorThreadPool());
      
      // store the operation engine
      mEngine = engine;
   }
   
   /**
    * Dispatches the next OperationExecutor in the queue that is available for
    * immediate execution.
    */
   @Override
   public void dispatchNextJob()
   {
      boolean dispatched = false;
      for(Iterator<Runnable> i = getJobIterator(); i.hasNext() && !dispatched;)
      {
         OperationExecutor executor = (OperationExecutor)i.next();
         
         // request execution from the engine
         if(mEngine.requestExecution(executor))
         {
            // set execution state
            executor.setExecutionState(OperationExecutionState.Executing);
            
            // remove the job from the queue
            i.remove();
            
            // run the job
            getThreadPool().runJob(executor);
            
            // job dispatched
            dispatched = true;
         }
      }
   }
}
