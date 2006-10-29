/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

import java.util.HashMap;

/**
 * An OperationEngineState represents the current state of an OperationEngine.
 * 
 * This object is checked before any OperationEngine can execute to ensure
 * that the OperationEngine is in an appropriate state for the execution of
 * the Operation.
 * 
 * This object is immutable. In order to change the state of an OperationEngine,
 * a MutableOperationEngineState object must be acquired.
 * 
 * @author Dave Longley
 */
public class OperationEngineState implements Cloneable
{
   /**
    * The underlying state variable table.
    */
   protected HashMap<String, Object> mVariableMap;
   
   /**
    * Creates a new, blank, OperationEngineState.
    */
   public OperationEngineState()
   {
      // create the variable map
      mVariableMap = new HashMap<String, Object>();
   }
   
   /**
    * Gets a state variable.
    * 
    * @param key the key for the variable.
    * 
    * @return the value of the variable.
    */
   public Object getVariable(String key)
   {
      return mVariableMap.get(key);
   }
   
   /**
    * Returns true if the state variable with the passed key exists,
    * false if not.
    * 
    * @param key the key for the variable.
    * 
    * @return true if the state variable with the passed key exists,
    *         false if not.
    */
   public boolean hasVariable(String key)
   {
      return mVariableMap.containsKey(key);
   }
   
   /**
    * Creates a copy of this state object.
    * 
    * @return a copy of this state object.
    */
   public OperationEngineState copy()
   {
      // create a new operation engine state
      OperationEngineState copy = new OperationEngineState();
      
      // copy all of the variables
      copy.mVariableMap.putAll(mVariableMap);
      
      return copy;
   }
   
   /**
    * Creates a copy of this state object.
    * 
    * @return a copy of this state object.
    */
   @Override
   public Object clone()
   {
      return copy();
   }
}
