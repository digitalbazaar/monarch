/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

/**
 * An OperationExecutionEnvironment defines an environment underwhich an
 * Operation can be executed. It checks the current state of an OperationEngine
 * by inspecting whatever state variables are relevant to the execution of
 * the Operation. It also updates the current state of an OperationEngine
 * if an Operation that uses this environment is to be executed.
 * 
 * Extending classes must provide the implementation that checks a passed
 * OperationEngineState to ensure that an Operation that relies on this
 * environment can be executed. They must also provide the implementation to
 * update a passed OperationEngineState when an Operation that uses this
 * environment is to be executed.
 * 
 * @author Dave Longley
 */
public abstract class OperationExecutionEnvironment
{
   /**
    * Creates a new OperationExecutionEnvironment.
    */
   public OperationExecutionEnvironment()
   {
   }
   
   /**
    * Determines if the passed OperationEngineState is appropriate for this
    * execution environment.
    * 
    * If it is, then an Operation that requires this execution environment
    * can be executed at some point (execution can occur immediately or after
    * a wait period).
    * 
    * If it is not, then an exception is thrown that indicates that the
    * OperationEngineState is inappropriate for the execution of an Operation
    * that requires this execution environment.
    * 
    * @param state the OperationEngineState to check.
    * 
    * @exception InappropriateOperationEngineState thrown if the passed
    *            state does not allow an Operation that requires this execution
    *            environment to be executed.
    */
   public abstract void checkEngineState(OperationEngineState state)
   throws InappropriateOperationEngineState;
   
   /**
    * Determines if the passed OperationEngineState is appropriate for the
    * immediate execution of an Operation or if an Operation must wait to
    * be executed.
    * 
    * @param state the OperationEngineState to check.
    * 
    * @return true if the passed state allows for the immediate execution of
    *         an Operation, false if an Operation must wait to be executed.
    */
   public abstract boolean canOperationExecuteImmediately(
      OperationEngineState state);
   
   /**
    * Updates the passed MutableOperationEngineState based on the passed
    * OperationExecutionStates.
    * 
    * @param oldExecutionState the old OperationExecutionState.
    * @param newExecutionState the new OperationExecutionState.
    * @param engineState the MutableOperationEngineState to update.
    */
   public abstract void updateEngineState(
      OperationExecutionState oldExecutionState,
      OperationExecutionState newExecutionState,
      MutableOperationEngineState engineState);
}
