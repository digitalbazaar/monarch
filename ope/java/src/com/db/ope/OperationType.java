/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

import java.util.List;

/**
 * An OperationType is an interface that defines a type of Operation. It
 * provides the methods necessary for an OperationEngine to execute an
 * Operation in a robust, multithreaded, safe fashion. A class that implements
 * this interface must define the environment underwhich an Operation can be
 * executed by an OperationEngine.
 * 
 * @author Dave Longley
 */
public interface OperationType
{
   /**
    * Called by an OperationEngine, that uses this OperationType, to determine
    * if an Operation can be executed. 
    * 
    * @param operation the Operation the engine seeks to execute.
    * @param currentOperations a list of the Operations that are currently
    *                          executing on the engine.
    * 
    * @return true if the passed Operation must wait before executing.
    */
   public boolean canExecute(Operation operation, List currentOperations);
   
   /**
    * Called by an OperationEngine, that uses this OperationType, to determine
    * if an Operation must wait before executing.
    * 
    * @param operation the Operation the engine seeks to execute.
    * @param currentOperations a list of the Operations that are currently
    *                          executing on the engine.
    * 
    * @return true if the passed Operation must wait before executing.
    */
   public boolean mustWait(Operation operation, List currentOperations);
}
