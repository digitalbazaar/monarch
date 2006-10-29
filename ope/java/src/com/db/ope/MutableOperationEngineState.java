/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.ope;

/**
 * An MutableOperationEngineState represents the current state of an
 * OperationEngine -- and allows for that state to be changed.
 * 
 * @author Dave Longley
 */
public class MutableOperationEngineState extends OperationEngineState
{
   /**
    * Creates a new, blank, MutableOperationEngineState.
    */
   public MutableOperationEngineState()
   {
   }
   
   /**
    * Sets a state variable. 
    * 
    * @param key the key for the variable.
    * @param value the value for the variable.
    */
   public void setVariable(String key, Object value)
   {
      mVariableMap.put(key, value);
   }
   
   /**
    * Clears all of the current state variables (removes them entirely).
    */
   public void clear()
   {
      mVariableMap.clear();
   }
}
