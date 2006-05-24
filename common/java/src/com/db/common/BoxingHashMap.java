/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import java.util.HashMap;
import java.util.Iterator;

/**
 * A BoxingHashMap is a hash map that boxes primitive types by creating
 * their Object counterparts and storing them in an internal hash map.
 * 
 * This class provides convenience methods for storing and retrieving
 * both primitive and object values.
 * 
 * @author Dave Longley
 */
public class BoxingHashMap extends HashMap
{
   /**
    * Creates a new, empty, BoxingHashMap.
    */
   public BoxingHashMap()
   {
   }
   
   /**
    * Puts a key-value pair in this map. The value is a byte and will
    * be boxed as a Byte.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    */
   public void put(Object key, byte value)
   {
      super.put(key, new Byte(value));
   }
   
   /**
    * Gets a value from a key in this map.
    * 
    * The value will be converted into a byte if possible. If the key
    * is not in the map, a NullPointerException will be thrown. If the value
    * cannot be cast to a byte, a ClassCastException will be thrown.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the value associated with the key as a byte.
    *         
    * @exception ClassCastException
    * @exception NullPointerException 
    */
   public byte getByte(Object key)
   throws ClassCastException, NullPointerException
   {
      Byte b = (Byte)get(key);
      return b.byteValue();
   }
   
   /**
    * Puts a key-value pair in this map. The value is a short and will
    * be boxed as a Short.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    */
   public void put(Object key, short value)
   {
      super.put(key, new Short(value));
   }
   
   /**
    * Gets a value from a key in this map.
    * 
    * The value will be converted into a short if possible. If the key
    * is not in the map, a NullPointerException will be thrown. If the value
    * cannot be cast to a short, a ClassCastException will be thrown.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the value associated with the key as a short.
    *         
    * @exception ClassCastException
    * @exception NullPointerException 
    */
   public short getShort(Object key)
   throws ClassCastException, NullPointerException
   {
      Short s = (Short)get(key);
      return s.shortValue();
   }

   /**
    * Puts a key-value pair in this map. The value is an integer and will
    * be boxed as an Integer.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    */
   public void put(Object key, int value)
   {
      super.put(key, new Integer(value));
   }
   
   /**
    * Gets a value from a key in this map.
    * 
    * The value will be converted into an int if possible. If the key
    * is not in the map, a NullPointerException will be thrown. If the value
    * cannot be cast to an int, a ClassCastException will be thrown.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the value associated with the key as an int.
    *         
    * @exception ClassCastException
    * @exception NullPointerException 
    */
   public int getInt(Object key)
   throws ClassCastException, NullPointerException
   {
      Integer i = (Integer)get(key);
      return i.intValue();
   }
   
   /**
    * Puts a key-value pair in this map. The value is a long and will
    * be boxed as a Long.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    */
   public void put(Object key, long value)
   {
      super.put(key, new Long(value));
   }
   
   /**
    * Gets a value from a key in this map.
    * 
    * The value will be converted into a long if possible. If the key
    * is not in the map, a NullPointerException will be thrown. If the value
    * cannot be cast to a long, a ClassCastException will be thrown.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the value associated with the key as a long.
    *         
    * @exception ClassCastException
    * @exception NullPointerException 
    */
   public long getLong(Object key)
   throws ClassCastException, NullPointerException
   {
      Long l = (Long)get(key);
      return l.longValue();
   }
   
   /**
    * Puts a key-value pair in this map. The value is a float and will
    * be boxed as a Float.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    */
   public void put(Object key, float value)
   {
      super.put(key, new Float(value));
   }
   
   /**
    * Gets a value from a key in this map.
    * 
    * The value will be converted into a float if possible. If the key
    * is not in the map, a NullPointerException will be thrown. If the value
    * cannot be cast to a float, a ClassCastException will be thrown.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the value associated with the key as a float.
    *         
    * @exception ClassCastException
    * @exception NullPointerException 
    */
   public float getFloat(Object key)
   throws ClassCastException, NullPointerException
   {
      Float f = (Float)get(key);
      return f.floatValue();
   }
   
   /**
    * Puts a key-value pair in this map. The value is a double and will
    * be boxed as a Double.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    */
   public void put(Object key, double value)
   {
      super.put(key, new Double(value));
   }
   
   /**
    * Gets a value from a key in this map.
    * 
    * The value will be converted into a double if possible. If the key
    * is not in the map, a NullPointerException will be thrown. If the value
    * cannot be cast to a double, a ClassCastException will be thrown.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the value associated with the key as a double.
    *         
    * @exception ClassCastException
    * @exception NullPointerException 
    */
   public double getDouble(Object key)
   throws ClassCastException, NullPointerException
   {
      Double d = (Double)get(key);
      return d.doubleValue();
   }
   
   /**
    * Puts a key-value pair in this map. The value is a boolean and will
    * be boxed as a Boolean.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    */
   public void put(Object key, boolean value)
   {
      super.put(key, new Boolean(value));
   }
   
   /**
    * Gets a value from a key in this map.
    * 
    * The value will be converted into a boolean if possible. If the key
    * is not in the map, a NullPointerException will be thrown. If the value
    * cannot be cast to a boolean, a ClassCastException will be thrown.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the value associated with the key as a boolean.
    *         
    * @exception ClassCastException
    * @exception NullPointerException 
    */
   public boolean getBoolean(Object key)
   throws ClassCastException, NullPointerException
   {
      Boolean b = (Boolean)get(key);
      return b.booleanValue();
   }
   
   /**
    * Puts a key-value pair in this map. The value is a char and will
    * be boxed as a Character.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    */
   public void put(Object key, char value)
   {
      super.put(key, new Character(value));
   }
   
   /**
    * Gets a value from a key in this map.
    * 
    * The value will be converted into a char if possible. If the key
    * is not in the map, a NullPointerException will be thrown. If the value
    * cannot be cast to a char, a ClassCastException will be thrown.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the value associated with the key as a char.
    *         
    * @exception ClassCastException
    * @exception NullPointerException 
    */
   public char getChar(Object key)
   throws ClassCastException, NullPointerException
   {
      Character c = (Character)get(key);
      return c.charValue();
   }
   
   /**
    * Puts a key-value pair in this map. The value is a String.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    */
   public void put(Object key, String value)
   {
      super.put(key, value);
   }
   
   /**
    * Gets a value from a key in this map.
    * 
    * The value will be converted into a String if possible. If the value
    * cannot be cast to a String, a ClassCastException will be thrown.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the value associated with the key as a String.
    *         
    * @exception ClassCastException
    */
   public String getString(Object key)
   throws ClassCastException
   {
      String s = (String)get(key);
      return s;
   }   
   
   /**
    * Copies data from the passed map into this map. This method makes deep
    * copies of boxed primitive values.
    * 
    * @param map the map to copy from.
    */
   public void copyFrom(BoxingHashMap map)
   {
      // copy key-value pairs
      Iterator i = map.keySet().iterator();
      while(i.hasNext())
      {
         Object key = i.next();
         Object value = map.get(key);
         
         Class type = value.getClass();
         
         if(type == Byte.class)
         {
            put(key, ((Byte)value).byteValue());
         }
         else if(type == Short.class)
         {
            put(key, ((Short)value).shortValue());
         }
         else if(type == Integer.class)
         {
            put(key, ((Integer)value).intValue());
         }
         else if(type == Long.class)
         {
            put(key, ((Long)value).longValue());
         }
         else if(type == Float.class)
         {
            put(key, ((Float)value).floatValue());
         }
         else if(type == Double.class)
         {
            put(key, ((Double)value).doubleValue());
         }
         else if(type == Boolean.class)
         {
            put(key, ((Boolean)value).booleanValue());
         }
         else if(type == Character.class)
         {
            put(key, ((Character)value).charValue());
         }
      }
   }

   /**
    * Copies this map to another map. This method makes deep copies of
    * boxed primitive values.
    * 
    * @return the copied map.
    */
   public BoxingHashMap copy()
   {
      BoxingHashMap copy = new BoxingHashMap();
      
      // copy values from this map
      copy.copyFrom(this);
      
      // return copy
      return copy;
   }
}
