/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

import com.db.util.MethodInvoker;

/**
 * An Operation represents some action that can be safely executed and
 * interrupted. It has an OperationType that defines a set of attributes
 * that an OperationEngine uses to determine the conditions underwhich the
 * Operation can be executed.
 * 
 * An Operation *must* be able to be interrupted and consequently terminate
 * cleanly. The OperationEngine that executes the Operation will wait
 * indefinitely for an interrupted Operation to terminate.
 * 
 * @author Dave Longley
 */
public class Operation
{
   /**
    * The environment underwhich this operation can execute.
    */
   protected OperationExecutionEnvironment mExecutionEnvironment;
   
   /**
    * The MethodInvoker used to execute this operation.
    */
   protected MethodInvoker mMethodInvoker;
   
   /**
    * The callback MethodInvoker used to execute a callback method after
    * this operation completes. It may be null.
    */
   protected MethodInvoker mCallbackInvoker;
   
   /**
    * Creates a new Operation with the given required execution environment and
    * the given MethodInvoker.
    * 
    * The passed MethodInvoker is used to invoke some method that contains
    * the actual code for this operation.
    * 
    * It should be understood that the invoker will be executed on an
    * OperationExecutorThread. That means that if the passed invoker has an
    * associated callback method, that callback method will be considered
    * part of the Operation and must comply with the requirements for
    * Operations.
    * 
    * If a callback method is required for this Operation that should not
    * be considered part of the operation, use the setCallbackInvoker() method
    * before passing this operation to an OperationEngine for execution. The
    * callback will be executed on an unsafe thread -- the management of that
    * thread is the responsibility of the caller of setCallbackInvoker().
    * 
    * @param environment the required execution environment for this operation.
    * @param invoker the methodInvoker to use to execute this operation.
    * 
    * @exception IllegalArgumentException thrown if the type or invoker is null.
    */
   public Operation(
      OperationExecutionEnvironment environment, MethodInvoker invoker)
   {
      if(environment == null)
      {
         throw new IllegalArgumentException(
            "OperationExecutionEnvironment cannot be null!");
      }
      
      if(invoker == null)
      {
         throw new IllegalArgumentException("MethodInvoker cannot be null!");
      }
      
      // store type and method invoker
      mExecutionEnvironment = environment;
      mMethodInvoker = invoker;
      
      // callback invoker is null by default
      mCallbackInvoker = null;
   }
   
   /**
    * Gets the execution environment underwhich this operation can execute.
    * 
    * @return the execution environment underwhich this operation can execute.
    */
   public OperationExecutionEnvironment getExecutionEnvironment()
   {
      return mExecutionEnvironment;
   }
   
   /**
    * Gets the MethodInvoker to use to execute this operation.
    * 
    * @return the MethodInvoker to use to execute this operation.
    */
   public MethodInvoker getMethodInvoker()
   {
      return mMethodInvoker;
   }
   
   /**
    * Sets a callback MethodInvoker for this operation. This callback will
    * executed on an unsafe thread -- whose management is the responsibility
    * of the caller of this method. If the callback for this Operation is
    * meant to be part of this Operation, then the callback should be set
    * in the MethodInvoker passed in the constructor. This method allows
    * a callback method to be associated with this Operation that is not
    * part of the actual Operation.
    * 
    * This method will only have an effect the first time it is called for
    * this Operation. Once a callback has been set for this Operation, it
    * cannot be changed.
    * 
    * @param cbInvoker the MethodInvoker to use as a callback when the
    *                  Operation finishes executing.
    */
   public void setCallbackInvoker(MethodInvoker cbInvoker)
   {
      if(mCallbackInvoker == null)
      {
         mCallbackInvoker = cbInvoker;
      }
   }
   
   /**
    * Gets a callback MethodInvoker for this operation. This callback will
    * executed on an unsafe thread -- whose management is the responsibility
    * of the caller of this method. If the callback for this Operation is
    * meant to be part of this Operation, then the callback should be set
    * in the MethodInvoker passed in the constructor. This method allows
    * a callback method to be associated with this Operation that is not
    * part of the actual Operation.
    * 
    * @return the MethodInvoker to use as a callback when the Operation
    *         finishes executing (can be null).
    */
   public MethodInvoker getCallbackInvoker()
   {
      return mCallbackInvoker;
   }
}
