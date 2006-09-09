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
    * Creates a new OperationExecutionResult for a given Operation. The passed
    * executor will either be the OperationExecutor instance used to execute
    * the Operation, or it will be null because the Operation cannot be
    * executed (the conditions necessary for its execution were not met).
    * 
    * @param executor the operation executor that executes the operation or
    *                 null if the operation cannot be executed.
    */
   public OperationExecutionResult(OperationExecutor executor)
   {
      // store executor
      mExecutor = executor;
   }
   
   /**
    * Interrupts the executing Operation, if the Operation is executing.
    */
   public void interrupt()
   {
      if(mExecutor != null)
      {
         mExecutor.interrupt();
      }
   }
   
   /**
    * Waits for the executing Operation to complete. This method causes
    * the current thread to wait for the Operation execution to complete.
    * 
    * @exception InterruptedException thrown if the current thread is
    *                                 interrupted while waiting for the
    *                                 Operation execution to complete.
    */
   public void waitForCompletion() throws InterruptedException
   {
      if(mExecutor != null)
      {
         mExecutor.waitForCompletion();
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
    * Returns true if the Operation is waiting to be executed, false if not.
    * 
    * @return true if the Operation is waiting to be executed, false if not.
    */
   public boolean isWaiting()
   {
      boolean rval = false;
      
      if(mExecutor != null)
      {
         rval = mExecutor.isWaiting();
      }
      
      return rval;
   }
   
   /**
    * Gets the return value for the operation.
    * 
    * @return the return value for the operation.
    * 
    * @exception IllegalStateException thrown if this method is called before
    *                                  the operation completes or if the
    *                                  operation could not be executed.
    */
   public Object getOperationReturnValue() throws IllegalStateException
   {
      Object rval = null;
      
      if(mExecutor != null)
      {
         if(mExecutor.hasFinished())
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
    *                                  the operation completes or if the
    *                                  operation could not be executed.
    */
   public Throwable getOperationException() throws IllegalStateException
   {
      Throwable rval = null;
      
      if(mExecutor != null)
      {
         if(mExecutor.hasFinished())
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
         throw new IllegalStateException(
            "The operation could not be executed so it cannot " +
            "have an exception."); 
      }
      
      return rval;
   }
}
