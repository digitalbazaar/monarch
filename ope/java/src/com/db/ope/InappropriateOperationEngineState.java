/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

/**
 * An InappropriateOperationEngineState is an exception that is thrown when
 * an OperationEngine is in a state that is not appropriate for the execution
 * of a particular Operation.
 * 
 * @author Dave Longley
 */
public class InappropriateOperationEngineState extends Exception
{
   /**
    * A copy of the OperationEngineState when this exception was created.
    */
   protected OperationEngineState mState;
   
   /**
    * Creates a new InappropriateOperationEngineState with no detail message.
    * 
    * The cause is not initialized, and may subsequently be initialized by a
    * call to initCause().
    * 
    * @param state a copy of the OperationEngineState.
    */
   public InappropriateOperationEngineState(OperationEngineState state)
   {
      this(state, null, null);
   }
   
   /**
    * Constructs a new InappropriateOperationEngineState with the specified
    * detail message. 
    * 
    * The cause is not initialized, and may subsequently be initialized by
    * a call to initCause().
    *
    * @param state a copy of the OperationEngineState.
    * @param message the detail message.
    * 
    */
   public InappropriateOperationEngineState(
      OperationEngineState state, String message)
   {
      this(state, message, null);
   }
   
   /**
    * Constructs a new InappropriateOperationEngineState with the specified
    * state and cause. The detail message will be null if the cause is null,
    * otherwise it will be the string representation of the cause.
    *
    * @param state a copy of the OperationEngineState.
    * @param cause the cause (null is permitted and indicates that the
    *              cause is nonexistent or unknown).
    */
   public InappropriateOperationEngineState(
      OperationEngineState state, Throwable cause)
   {
      this(state, null, cause);
   }
   
   /**
    * Constructs a new exception with the specified state, detail message, and
    * cause.
    *
    * @param state a copy of the OperationEngineState.
    * @param message the detail message.
    * @param cause the cause (null is permitted and indicates that the
    *              cause is nonexistent or unknown).
    */
   public InappropriateOperationEngineState(
      OperationEngineState state, String message, Throwable cause)
   {
      // initialize message and cause
      super(message, cause);
      
      // store the state
      mState = state;
   }
   
   /**
    * Gets the copy of the OperationEngineState that was generated when this
    * exception was created.
    * 
    * @return the copy OperationEngineState that was generated when this 
    *         exception was created.
    */
   public OperationEngineState getState()
   {
      return mState;
   }   
}
