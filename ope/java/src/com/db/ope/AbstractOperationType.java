/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

import java.util.List;

import com.db.util.UniqueSet;

/**
 * An AbstractOperationType provides convenience functionality for defining
 * an environment underwhich an Operation can be run.
 * 
 * For example, it provides methods for storing:
 * 
 * 1. which OperationTypes' Operations will block the execution of an Operation
 *    of this OperationType,
 * 2. which OperationType's Operations will be blocked by the execution of
 *    an Operation of this OperationType,
 * 3. and which OperationTypes' Operations must be executed synchronously with
 *    Operations of this OperationType.
 * 
 * @author Dave Longley
 */
public abstract class AbstractOperationType
{
   /**
    * A set of OperationTypes that prevent (entirely) an Operation of this
    * OperationType from executing. 
    */
   protected UniqueSet mBlockingOperationTypes;
   
   /**
    * A set of OperationTypes whose Operations an executing Operation of this
    * OperationType will prevent (entirely) from executing.
    */
   protected UniqueSet mBlockedOperationTypes;
   
   /**
    * A set of OperationTypes that require an Operation of this OperationType
    * to wait while other Operations execute (and vice versa). Operations of
    * this OperationType and an OperationType in this set must execute
    * synchronously.  
    */
   protected UniqueSet mSynchronousOperationTypes;
   
   /**
    * Creates a new AbstractOperationType.
    */
   public AbstractOperationType()
   {
      // create the OperationType sets
      mBlockingOperationTypes = new UniqueSet();
      mBlockedOperationTypes = new UniqueSet();
      mSynchronousOperationTypes = new UniqueSet();
   }
   
   /**
    * Adds an OperationType that prevents (entirely) an Operation of this
    * OperationType from executing.
    * 
    * @param type the OperationType to add.
    */
   public void addBlockingOperationType(OperationType type)
   {
      mBlockingOperationTypes.add(type);
   }
   
   /**
    * Adds an OperationType whose Operations this OperationType prevents
    * (entirely) from executing.
    * 
    * @param type the OperationType to add.
    */
   public void addBlockedOperationType(OperationType type)
   {
      mBlockedOperationTypes.add(type);
   }
   
   /**
    * Adds an OperationType whose Operations must be executed synchronously
    * with the Operations of this OperationType.
    * 
    * @param type the OperationType to add.
    */
   public void addSynchronousOperationType(OperationType type)
   {
      mSynchronousOperationTypes.add(type);
   }
   
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
   public boolean canExecute(Operation operation, List currentOperations)
   {
      boolean rval = false;
      
      // FIXME: implement me
      
      return rval;
   }
   
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
   public boolean mustWait(Operation operation, List currentOperations)
   {
      boolean rval = false;
      
      // FIXME: implement me
      
      return rval;
   }
}
