/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

/**
 * An OperationExecutor is a class that executes an Operation for an
 * OperationEngine.
 * 
 * @author Dave Longley
 */
public class OperationExecutor implements Runnable
{
   /**
    * The Operation that is executed by this OperationExecutor.
    */
   protected Operation mOperation;
   
   /**
    * Set to true if this OperationExecutor has started executing its
    * Operation, false if not.
    */
   protected boolean mStarted;
   
   /**
    * Set to true if the Operation has been interrupted, false if not.
    */
   protected boolean mInterrupted;
   
   /**
    * The thread this OperationExecutor is running on. This can be null if
    * it has not been set yet because the executor's run() method hasn't
    * been called yet.
    */
   protected Thread mExecutionThread;
   
   /**
    * Set to true once the Operation's execution has completed, false otherwise.
    */
   protected boolean mExecutionCompleted;
   
   /**
    * Creates a new OperationExecutor.
    * 
    * @param operation the Operation to execute.
    */
   public OperationExecutor(Operation operation)
   {
      // store the operation
      mOperation = operation;
      
      // operation not interrupted yet
      mInterrupted = false;
      
      // no execution thread yet
      mExecutionThread = null;
      
      // execution not completed yet
      mExecutionCompleted = false;
   }
   
   /**
    * Executes the Operation.
    */
   public void run()
   {
      // lock while setting the execution thread and checking for interruption
      synchronized(this)
      {
         // store the execution thread as the current thread
         mExecutionThread = Thread.currentThread();
         
         // determine if the operation has been interrupted, if so,
         // the execution thread must be interrupted
         if(isInterrupted())
         {
            // interrupt the execution thread
            mExecutionThread.interrupt();
         }
      }
      
      // execute the operation's method invoker
      mOperation.getMethodInvoker().execute();
      
      // call the operation's callback, if one exists, on another thread --
      // an unsafe thread, it is not managed here
      if(mOperation.getCallbackInvoker() != null)
      {
         mOperation.getCallbackInvoker().backgroundExecute();
      }
      
      // operation execution completed
      mExecutionCompleted = true;
   }
   
   /**
    * Interrupts the executing Operation.
    */
   public synchronized void interrupt()
   {
      // sets the operation to interrupted
      mInterrupted = true;
      
      // see if the execution thread has been set yet
      if(mExecutionThread != null)
      {
         // interrupt the execution thread
         mExecutionThread.interrupt();
      }
   }
   
   /**
    * Returns true if the executing Operation has been interrupted, false
    * if not.
    * 
    * @return true if the executing Operation has been interrupted, false
    *         if not.
    */
   public synchronized boolean isInterrupted()
   {
      return mInterrupted;
   }
   
   
   /**
    * Causes the current thread to wait for the Operation execution to
    * complete.
    * 
    * @exception InterruptedException thrown if the current thread is
    *                                 interrupted while waiting for the
    *                                 Operation execution to complete.
    */
   public void waitForCompletion() throws InterruptedException
   {
      while(!mExecutionCompleted)
      {
         Thread.sleep(1);
      }
   }
   
   /**
    * Sets whether or not this OperationExecutor has started executing
    * its Operation.
    * 
    * @param started true if this OperationExecutor has started executing
    *                its Operation, false if not.
    */
   public synchronized void setStarted(boolean started)
   {
      mStarted = started;
   }
   
   /**
    * Gets whether or not this OperationExecutor has started executing
    * its Operation. 
    * 
    * @return true if this OperationExecutor has started executing its
    *         Operation, false if not.
    */
   public synchronized boolean hasStarted()
   {
      return mStarted;
   }
   
   /**
    * Returns true if this OperationExecutor has finished executing its
    * Operation, false if not.
    * 
    * @return true if this OperationExecutor has finished executing its
    *         Operation, false if not.
    */
   public synchronized boolean hasFinished()
   {
      return mExecutionCompleted;
   }
}
