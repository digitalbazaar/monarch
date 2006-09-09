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
}
