/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

import java.util.Iterator;
import java.util.List;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.util.JobDispatcher;
import com.db.util.UniqueSet;

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
 * This engine accomplishes this by using the OperationTypes that are added
 * to this engine. An OperationType defines the environment underwhich an
 * Operation is allowed to run. This engine handles the synchronization of the
 * operations as well as monitors the threads different operations are running
 * on and ensures that the operations can be interrupted and cleaned up nicely,
 * provided that those classes that use Operations and define OperationTypes
 * comply with the Operation requirements. 
 * 
 * @author Dave Longley
 */
public class OperationEngine
{
   /**
    * The set of OperationTypes supported by this engine. 
    */
   protected UniqueSet mOperationTypes;
   
   /**
    * The JobDispatcher for dispatching OperationExecutors. This dispatcher
    * is used to store OperationExecutors in a queue and dispatch them for
    * execution when they can be executed. An OperationExecutor can only
    * be pushed onto the queue for this dispatcher once the conditions
    * necessary for allowing it to run have been met.
    */
   protected JobDispatcher mDispatcher;
   
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
      // create the operation types set
      mOperationTypes = new UniqueSet();
      
      // create the operation executor thread pool
      OperationExecutorThreadPool threadPool =
         new OperationExecutorThreadPool();
      
      // create the operation executor dispatcher
      mDispatcher = new JobDispatcher(threadPool);
   }
   
   /**
    * Determines if the passed Operation can be executed by this engine. This
    * method will first see if the Operation's type is even supported by this
    * engine, and then it will check all of the supported OperationTypes to
    * ensure that the passed Operation can be executed.
    * 
    * @param operation the Operation this engine seeks to execute.
    * 
    * @return true if the passed Operation can be executed, false if not.
    */
   protected synchronized boolean canExecute(Operation operation)
   {
      boolean rval = true;
      
      for(Iterator i = mOperationTypes.iterator(); i.hasNext() && rval;)
      {
         // FIXME: implement me, use correct operation list
         OperationType type = (OperationType)i.next();
         rval = type.canExecute(operation, getOperationList());
      }
      
      return rval;
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
    * Gets an immutable list of the currently executing Operations. This
    * list does not include Operations that are waiting to execute.
    * 
    * @return an immutable list of the currently executing Operations.
    */
   protected List getOperationList()
   {
      List rval = null;
      
      // FIXME: implement me, when getting this list, remove the
      // OperationExecutors that are completed (lazy cleanup that should
      // always work since this list must be fetched to start a new
      // operation)
      
      return rval;
   }
   
   /**
    * Adds an OperationType to this engine. OperationTypes can only be
    * added to this engine when it is not running.
    * 
    * @param type the operation type to add to this engine.
    */
   public synchronized void addOperationType(OperationType type)
   {
      // only consider adding an OperationType if the engine is not running
      if(!isRunning())
      {
         // add the operation type
         mOperationTypes.add(type);
      }
   }
   
   /**
    * Removes an OperationType from this engine. OperationTypes can only be
    * removed from this engine when it is not running.
    * 
    * @param type the operation type to remove from this engine.
    */
   public synchronized void removeOperationType(OperationType type)
   {
      // only consider removing an OperationType if the engine is not running
      if(!isRunning())
      {
         // remove the operation type
         mOperationTypes.remove(type);
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
         
         // FIXME: implement me
         
         // start dispatching operation executors
         mDispatcher.startDispatching();
         
         // FIXME: implement me
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
         
         // FIXME: implement me
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
    */
   public synchronized OperationExecutionResult execute(Operation operation)
   {
      OperationExecutionResult rval = null;

      // determine if the operation can be executed
      if(canExecute(operation))
      {
         // create an OperationExecutor
         OperationExecutor executor = new OperationExecutor(this, operation);
      
         // create a new OperationExecutionResult using the executor
         rval = new OperationExecutionResult(executor);
         
         // FIXME: add the operation to the appropriate lists
         // i.e. execution list, wait list?
         
         // queue the OperationExecutor with the dispatcher
         mDispatcher.queueJob(executor);
      }
      else
      {
         // the operation cannot be executed, so create a new
         // OperationExecutionResult that has no executor
         rval = new OperationExecutionResult(null);
      }
      
      return rval;
   }
   
   /**
    * Returns whether or not the passed OperationExecutor must wait before
    * executing.
    * 
    * @param executor the OperationExecutor to check.
    * 
    * @return true if the OperationExecutor must wait to execute, false if not.
    */
   public synchronized boolean mustWait(OperationExecutor executor)
   {
      boolean rval = false;
      
      // FIXME: implement me
      for(Iterator i = mOperationTypes.iterator(); i.hasNext() && !rval;)
      {
         // FIXME: implement me, use correct operation list
         OperationType type = (OperationType)i.next();
         rval = type.mustWait(executor.getOperation(), getOperationList());
      }
      
      // FIXME:
      // if the executor must wait, then add it to the list of waiting
      // executors that require notification to wakeup
      
      return rval;
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
