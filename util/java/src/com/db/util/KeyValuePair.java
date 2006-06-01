/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.util.Comparator;

/**
 * A simple class for storing key-value pairs.
 * 
 * @author Dave Longley
 */
public class KeyValuePair implements Comparator
{
   /**
    * The key.
    */
   protected Object mKey;
   
   /**
    * The value.
    */
   protected Object mValue;
   
   /**
    * Creates a new KeyValuepair with the given key and value.
    * 
    * @param key the key.
    * @param value the value.
    */
   public KeyValuePair(Object key, Object value)
   {
      mKey = key;
      mValue = value;
   }
   
   /**
    * Returns the key.
    *
    * @return the key.
    */
   public Object getKey()
   {
      return mKey;
   }
   
   /**
    * Returns the value.
    * 
    * @return the value.
    */
   public Object getValue()
   {
      return mValue;
   }
   
   /**
    * Compares two key-value pairs. Compares on the value of the key-value
    * pair. If the first argument has a lesser value than the second,
    * a negative integer is returned. If the first argument is equal to
    * the second, 0 is returned. If the first argument is greater than
    * the second, than a positive integer is returned.
    * 
    * @param o1 the first object to be compared.
    * @param o2 the second object to be compared.
    * @return a negative integer, zero, or a positive integer as the
    *         first argument is less than, equal to, or greater than the
    *         second. 
    * @throws ClassCastException if the arguments' types prevent them from
    *          being compared by this Comparator.
    */
   public int compare(Object o1, Object o2)
   {
      int rval = 0;
      
      KeyValuePair kvp1 = (KeyValuePair)o1;
      KeyValuePair kvp2 = (KeyValuePair)o2;
      
      String value1 = kvp1.getValue().toString();
      String value2 = kvp2.getValue().toString();
      
      rval = value1.compareTo(value2);
      
      if(rval != 0)
      {
         rval = (rval < 0) ? -1 : 1;
      }
      
      return rval;
   }   
   
   /**
    * A key-value pair is equal to another key-value pair if
    * its key and value are equal.
    * 
    * @param obj another key-value pair to compare to.
    */
   public boolean equals(Object obj)
   {
      boolean rval = false;
      
      if(obj != this)
      {
         /*if(obj != null)
         {
            if(obj instanceof KeyValuePair)
            {
               KeyValuePair kvp = (KeyValuePair)obj;
               if(kvp.getKey().equals(getKey()) &&
                  kvp.getValue().equals(getValue()))
               {
                  rval = true;
               }
            }
         }*/
      }
      else
      {
         rval = true;
      }
      
      return rval;
   }
}
