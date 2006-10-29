/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

import com.db.logging.Logger;
import com.db.util.MethodInvokedMessage;

/**
 * An OperationExecutor is a class that executes an Operation for an
 * OperationEngine.
 * 
 * @author Dave Longley
 */
public class OperationExecutor implements Runnable
{
   /**
    * The OperationEngine this OperationExecutor is for.
    */
   protected OperationEngine mEngine;
   
   /**
    * The Operation that is executed by this OperationExecutor.
    */
   protected Operation mOperation;
   
   /**
    * The Operation's current execution state.
    */
   protected OperationExecutionState mExecutionState;
   
   /**
    * The thread this OperationExecutor is running on. This can be null if
    * it has not been set yet because the executor's run() method hasn't
    * been called yet.
    */
   protected Thread mExecutionThread;
   
   /**
    * Creates a new OperationExecutor.
    * 
    * @param engine the OperationEngine this OperationExecutor is for.
    * @param operation the Operation to execute.
    */
   public OperationExecutor(OperationEngine engine, Operation operation)
   {
      // store the engine
      mEngine = engine;
      
      // store the operation
      mOperation = operation;
      
      // no execution state
      mExecutionState = OperationExecutionState.None;
      
      // no execution thread yet
      mExecutionThread = null;
   }
   
   /**
    * Executes the Operation.
    */
   public void run()
   {
      // lock while setting the execution thread and
      // checking for interruption
      synchronized(this)
      {
         // store the execution thread as the current thread
         mExecutionThread = Thread.currentThread();
         
         // determine if the operation has been interrupted, if so,
         // the execution thread must be interrupted
         if(getExecutionState() == OperationExecutionState.Interrupted)
         {
            // interrupt the execution thread
            mExecutionThread.interrupt();
         }
      }
      
      // execute the operation's method invoker
      getOperation().getMethodInvoker().execute();
      
      // see if an exception was thrown
      Throwable t = getMethodInvokedMessage().getMethodException();
      
      // see if the operation was interrupted
      if(t instanceof InterruptedException ||
         (t.getCause() != null && t.getCause() instanceof InterruptedException))
      {
         // maintain interrupted status
         interrupt();
         
         // handle uncaught interruption
         getLogger().debug(getClass(), "Operation interrupted.");
      }
      else
      {
         // set execution state
         setExecutionState(OperationExecutionState.Completed);
      }
      
      // notify engine of end of execution
      mEngine.executionStopped(this);
      
      // call the operation's callback, if one exists, on another thread --
      // an unsafe thread, it is not managed here
      if(getOperation().getCallbackInvoker() != null)
      {
         getOperation().getCallbackInvoker().backgroundExecute();
      }
   }
   
   /**
    * Interrupts the executing Operation.
    */
   public synchronized void interrupt()
   {
      // set execution state
      setExecutionState(OperationExecutionState.Interrupted);
      
      // see if the execution thread has been set yet
      if(mExecutionThread != null)
      {
         // interrupt the execution thread
         mExecutionThread.interrupt();
      }
   }
   
   /**
    * Waits until Operation execution is completed or interrupted. This method
    * causes the current thread to wait for the Operation execution to stop.
    * 
    * @exception InterruptedException thrown if the current thread is
    *            interrupted while waiting for the Operation execution to stop.
    */
   public void waitWhileExecuting() throws InterruptedException   
   {
      while(getExecutionState() != OperationExecutionState.Completed &&
            getExecutionState() != OperationExecutionState.Interrupted)
      {
         Thread.sleep(1);
      }
   }
   
   /**
    * Returns true if Operation execution has stopped due to Operation
    * completion or interruption. 
    * 
    * @return true if Operation execution has stopped due to Operation
    *         completion or interruption.
    */
   public boolean hasStopped()
   {
      boolean rval = false;
      
      if(getExecutionState() != OperationExecutionState.Completed &&
         getExecutionState() != OperationExecutionState.Interrupted)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Sets the execution state for this OperationExecutor.
    * 
    * @param state the execution state.
    */
   public synchronized void setExecutionState(OperationExecutionState state)
   {
      mExecutionState = state;
   }
   
   /**
    * Gets the execution state for this OperationExecutor.
    * 
    * @return state the execution state.
    */
   public synchronized OperationExecutionState getExecutionState()
   {
      return mExecutionState;
   }
   
   /**
    * Gets the Operation that is executed by this OperationExecutor.
    * 
    * @return the Operation that is executed by this OperationExecutor.
    */
   public Operation getOperation()
   {
      return mOperation;
   }
   
   /**
    * Gets the MethodInvokedMessage for the method that was executed for
    * the Operation.
    * 
    * @return the MethodInvokedMessage for the method executed for the
    *         Operation.
    */
   public MethodInvokedMessage getMethodInvokedMessage()
   {
      return getOperation().getMethodInvoker().getMessage();
   }
   
   /**
    * Gets the logger for this OperationExecutor.
    * 
    * @return the logger for this OperationExecutor.
    */
   public Logger getLogger()
   {
      return mEngine.getLogger();
   }
}
