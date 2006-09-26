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
 * For example, it provides methods for setting:
 * 
 * 1. which OperationTypes' Operations will block the execution of an Operation
 *    of this OperationType,
 * 2. which OperationType's Operations will be blocked by the execution of
 *    an Operation of this OperationType,
 * 3. which OperationTypes' Operations must be executed synchronously with
 *    Operations of this OperationType,
 * 4. and the maximum number of concurrent Operations of this OperationType.
 * 
 * @author Dave Longley
 */
public abstract class AbstractOperationType
{
   /**
    * A set of OperationTypes that prevent (entirely) an Operation of this
    * OperationType from executing. 
    */
   protected UniqueSet<OperationType> mBlockingOperationTypes;
   
   /**
    * A set of OperationTypes whose Operations an executing Operation of this
    * OperationType will prevent (entirely) from executing.
    */
   protected UniqueSet<OperationType> mBlockedOperationTypes;
   
   /**
    * A set of OperationTypes that require an Operation of this OperationType
    * to wait while other Operations execute (and vice versa). Operations of
    * this OperationType and an OperationType in this set must execute
    * synchronously.  
    */
   protected UniqueSet<OperationType> mSynchronousOperationTypes;
   
   /**
    * The maximum number of Operations of this OperationType that can run
    * concurrently. A value of 0 indicates there is no limitation.
    */
   protected int mMaxConcurrentOperations;
   
   /**
    * Creates a new AbstractOperationType.
    */
   public AbstractOperationType()
   {
      // create the OperationType sets
      mBlockingOperationTypes = new UniqueSet<OperationType>();
      mBlockedOperationTypes = new UniqueSet<OperationType>();
      mSynchronousOperationTypes = new UniqueSet<OperationType>();
      
      // set no limitation on the number of concurrent operations
      setMaxConcurrentOperations(0);
   }
   
   /**
    * Adds an OperationType that prevents (entirely) an Operation of this
    * OperationType from executing.
    * 
    * @param type the OperationType to add.
    */
   public synchronized void addBlockingOperationType(OperationType type)
   {
      mBlockingOperationTypes.add(type);
   }
   
   /**
    * Adds an OperationType whose Operations this OperationType prevents
    * (entirely) from executing.
    * 
    * @param type the OperationType to add.
    */
   public synchronized void addBlockedOperationType(OperationType type)
   {
      mBlockedOperationTypes.add(type);
   }
   
   /**
    * Adds an OperationType whose Operations must be executed synchronously
    * with the Operations of this OperationType.
    * 
    * @param type the OperationType to add.
    */
   public synchronized void addSynchronousOperationType(OperationType type)
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
   public synchronized boolean canExecute(
      Operation operation, List currentOperations)
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
   public synchronized boolean mustWait(
      Operation operation, List currentOperations)
   {
      boolean rval = false;
      
      // FIXME: implement me
      
      return rval;
   }
   
   /**
    * Sets the maximum number of Operations of this OperationType that can run
    * concurrently. A value of 0 specifies no limitation.
    * 
    * @param max the maximum number of Operations of this OperationType that
    *            can run concurrently, a value of 0 indicating no limitation.
    */
   public synchronized void setMaxConcurrentOperations(int max)
   {
      mMaxConcurrentOperations = Math.max(0, max); 
   }
   
   /**
    * Gets the maximum number of Operations of this OperationType that can run
    * concurrently. A value of 0 specifies no limitation.
    * 
    * @return the maximum number of Operations of this OperationType that
    *         can run concurrently, a value of 0 indicating no limitation.
    */
   public synchronized int getMaxConcurrentOperations()
   {
      return mMaxConcurrentOperations;
   }
}
