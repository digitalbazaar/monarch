/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

/**
 * An OperationExecutionResult represents the result of executing an Operation
 * on an OperationEngine. It provides an interface for interrupting or
 * terminating the Operation and for obtaining information about the execution
 * status.
 * 
 * This class is hides away lower-level functionality that should not be
 * accessible to the application that is running operations.
 * 
 * @author Dave Longley
 */
public class OperationExecutionResult
{
   /**
    * The OperationExecutor that is executing the operation.
    */
   protected OperationExecutor mExecutor;
   
   /**
    * Any exception that was thrown before the operation started execution. 
    */
   protected Exception mException;
   
   /**
    * Creates a new OperationExecutionResult for an Operation. The passed
    * executor is the OperationExecutor instance used to execute the Operation.
    * 
    * @param executor the operation executor that executes the operation.
    */
   public OperationExecutionResult(OperationExecutor executor)
   {
      // store executor
      mExecutor = executor;
      
      // no exception yet
      mException = null;
   }   
   
   /**
    * Creates a new OperationExecutionResult for an Operation that could
    * not be executed. The passed exception explains why the Operation
    * could not be executed.
    * 
    * @param e the exception that explains why the Operation could not be
    *          executed.
    */
   public OperationExecutionResult(Exception e)
   {
      // no executor
      mExecutor = null;
      
      // the exception
      mException = e;
   }
   
   /**
    * Interrupts the Operation whether it is waiting to execute or is executing.
    */
   public void interrupt()
   {
      if(mExecutor != null)
      {
         mExecutor.interrupt();
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
      if(mExecutor != null)
      {
         mExecutor.waitWhileExecuting();
      }
   }
   
   /**
    * Returns true if the Operation could be executed, false if not.
    * 
    * @return true if the Operation could be executed, false if not.
    */
   public boolean couldExecute()
   {
      return mExecutor != null;
   }
   
   /**
    * Gets the return value for the operation.
    * 
    * @return the return value for the operation.
    * 
    * @exception IllegalStateException thrown if this method is called before
    *            the operation completes or if the operation could not be
    *            executed.
    */
   public Object getReturnValue() throws IllegalStateException
   {
      Object rval = null;
      
      if(mExecutor != null)
      {
         if(mExecutor.hasStopped())
         {
            rval = mExecutor.getMethodInvokedMessage().getMethodReturnValue();
         }
         else
         {
            throw new IllegalStateException(
               "The operation has not finished executing, so it cannot " +
               "have a return value yet.");
         }
      }
      else
      {
         throw new IllegalStateException(
            "The operation could not be executed so it cannot " +
            "have a return value."); 
      }
      
      return rval;
   }
   
   /**
    * Gets an exception that was thrown while executing the operation.
    * 
    * @return the exception thrown while executing the operation, can be null.
    * 
    * @exception IllegalStateException thrown if this method is called before
    *            the operation completes.
    */
   public Throwable getException() throws IllegalStateException
   {
      Throwable rval = null;
      
      if(mExecutor != null)
      {
         if(mExecutor.hasStopped())
         {
            rval = mExecutor.getMethodInvokedMessage().getMethodException();
         }
         else
         {
            throw new IllegalStateException(
               "The operation has not finished executing, so it cannot " +
               "have an exception yet.");
         }
      }
      else
      {
         return mException; 
      }
      
      return rval;
   }
}
