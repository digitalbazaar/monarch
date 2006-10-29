/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

/**
 * An OperationExecutionState enumerates the execution states for an Operation.
 * 
 * @author Dave Longley
 */
public enum OperationExecutionState
{
   /**
    * When an Operation has no execution state.
    */
   None,
   
   /**
    * When Operation execution is waiting.
    */
   Waiting,
   
   /**
    * When an Operation is executing.
    */
   Executing,
   
   /**
    * When Operation execution has been interrupted.
    */
   Interrupted,
   
   /**
    * When Operation execution has been completed.
    */
   Completed;
}
