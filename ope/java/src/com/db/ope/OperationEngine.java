/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * An OperationEngine is a processing engine that executes Operations. The
 * purpose of this engine is to create a robust, multithreaded environment in
 * which these Operations can be safely executed and interrupted.
 * 
 * Furthermore, it is designed with the intention of taking care of a lot of
 * the synchronization headaches that a programmer must consider when designing
 * a complex set of behaviors that must work in conjunction with other complex
 * behaviors -- while none of those behaviors really need to know all that much
 * about one another.
 * 
 * This engine accomplishes this requiring that every Operation have an
 * OperationExecutionEnvironment that describes the conditions underwhich the
 * Operation can execute. Before any Operation can be queued for execution,
 * its OperationExecutionEnvironment checks the current OperationEngineState
 * to ensure that the Operation can be executed.
 * 
 * @author Dave Longley
 */
public class OperationEngine
{
   /**
    * The current state of this OperationEngine.
    */
   protected MutableOperationEngineState mState;
   
   /**
    * The OperationExecutorDispatcher for dispatching OperationExecutors. This
    * dispatcher is used to store OperationExecutors in a queue and dispatch
    * them for execution when they can be executed. An OperationExecutor can
    * only be pushed onto the queue for this dispatcher once the conditions
    * necessary for allowing it to run have been met.
    */
   protected OperationExecutorDispatcher mDispatcher;
   
   /**
    * Set to true when this OperationEngine is running, false when it
    * is not.
    */
   protected boolean mRunning;
   
   /**
    * Creates a new OperationEngine.
    */
   public OperationEngine()
   {
      // create the state for this engine
      mState = new MutableOperationEngineState();
      
      // create the operation executor dispatcher
      mDispatcher = new OperationExecutorDispatcher(this);
   }
   
   /**
    * Sets whether or not this OperationEngine is running.
    * 
    * @param running true if this OperationEngine is running, false if not.
    */
   protected synchronized void setRunning(boolean running)
   {
      mRunning = running;
   }
   
   /**
    * Requests immediate execution of an Operation. If the Operation can be
    * immediately executed, then the state of this engine is updated
    * appropriately.
    * 
    * @param executor the OperationExecutor requesting execution.
    * 
    * @return true if the Operation can be executed immediately, false if not.
    */
   protected synchronized boolean requestExecution(OperationExecutor executor)
   {
      boolean rval = false;
      
      // lock on the engine state
      synchronized(getState())
      {
         // get the execution environment for the operation
         OperationExecutionEnvironment environment =
            executor.getOperation().getExecutionEnvironment();
         
         // see if the operation can be executed immediately
         if(environment.canOperationExecuteImmediately(getState()))
         {
            // update the engine state
            environment.updateEngineState(
               executor.getExecutionState(),
               OperationExecutionState.Executing,
               mState);
            
            // execution granted
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Called when an Operation has completed or been interrupted to update the
    * state of this engine.
    * 
    * @param executor the OperationExecutor that completed execution or was
    *                 interrupted.
    */
   protected synchronized void executionStopped(OperationExecutor executor)   
   {
      // lock on the engine state
      synchronized(getState())      
      {
         // get the execution environment for the operation
         OperationExecutionEnvironment environment =
            executor.getOperation().getExecutionEnvironment();
         
         // update the engine state
         environment.updateEngineState(
            OperationExecutionState.Executing,
            executor.getExecutionState(),
            mState);      
      }
   }
   
   /**
    * Starts this OperationEngine, if it is not already running. This engine
    * cannot execute Operations until it has been started.
    * 
    * This method will have no affect if the engine is already running.
    */
   public synchronized void start()
   {
      // start engine if it is not already running
      if(!isRunning())
      {
         // now running
         setRunning(true);
         
         // clear current state
         mState.clear();
         
         // start dispatching operation executors
         mDispatcher.startDispatching();
      }
   }
   
   /**
    * Stops this OperationEngine, if it is currently running. This engine
    * cannot execute Operations when it is stopped.
    * 
    * This method will have no affect if the engine is not running.
    */
   public synchronized void stop()
   {
      // stop engine if it is currently running
      if(isRunning())
      {
         // no longer running
         setRunning(false);
         
         // stop dispatching jobs
         mDispatcher.stopDispatching();
         
         // clear the queue of jobs to dispatch
         mDispatcher.clearQueuedJobs();
         
         // terminate all running jobs (wait indefinitely -- operations *must*
         // terminate when they have been interrupted) 
         mDispatcher.terminateAllRunningJobs();
      }
   }
   
   /**
    * Returns true if this OperationEngine is running, false if not.
    * 
    * @return true if this OperationEngine is running, false if not.
    */
   public synchronized boolean isRunning()
   {
      return mRunning;
   }
   
   /**
    * Executes the passed Operation.
    * 
    * @param operation the Operation to execute.
    * 
    * @return the OperationExecutionResult for the execution.
    * 
    * @exception IllegalStateException thrown if the OperationEngine is not
    *            running when this method is called.
    */
   public synchronized OperationExecutionResult execute(Operation operation)
   {
      OperationExecutionResult rval = null;
      
      if(isRunning())
      {
         try
         {
            // get a lock on the current engine state
            synchronized(getState())
            {
               // get the execution environment for the operation
               OperationExecutionEnvironment environment =
                  operation.getExecutionEnvironment();
               
               // check the current engine state against the execution
               // environment for the operation
               environment.checkEngineState(getState());
               
               // create an OperationExecutor
               OperationExecutor executor =
                  new OperationExecutor(this, operation);
               
               // create a new OperationExecutionResult using the executor
               rval = new OperationExecutionResult(executor);
               
               // update the engine state
               environment.updateEngineState(
                  executor.getExecutionState(),
                  OperationExecutionState.Waiting,
                  mState);
               
               // update execution state
               executor.setExecutionState(OperationExecutionState.Waiting);
               
               // queue the OperationExecutor with the dispatcher
               mDispatcher.queueJob(executor);
            }
         }
         catch(InappropriateOperationEngineState e)
         {
            // create a new OperationExecutionResult using the exception
            rval = new OperationExecutionResult(e);
         }
      }
      else
      {
         throw new IllegalStateException(
            "The OperationEngine is not running so no Operations can be " +
            "executed.");         
      }
      
      return rval;
   }
   
   /**
    * Gets the current state of this engine in an immutable form.
    * 
    * @return the current state of this engine in an immutable form.
    */
   public OperationEngineState getState()
   {
      return mState;
   }
   
   /**
    * Gets the logger for this OperationEngine.
    * 
    * @return the logger for this OperationEngine.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbope");
   }
}
