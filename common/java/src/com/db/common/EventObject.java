/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

/**
 * A generic event object for passing event data.
 * 
 * @author Dave Longley
 */
public class EventObject
{
   /**
    * The name of this event.
    */
   protected String mName;
   
   /**
    * A map of data bundled with this event.
    */
   protected BoxingHashMap mDataMap;
   
   /**
    * Creates a new EventObject with no specified name.
    */
   public EventObject()
   {
      this("");
   }

   /**
    * Creates a new EventObject with the given name.
    * 
    * @param name the name of this event.
    */
   public EventObject(String name)
   {
      setName(name);
      mDataMap = new BoxingHashMap();
   }

   /**
    * Sets the name of this event.
    * 
    * @param name the name of this event.
    */
   public void setName(String name)
   {
      mName = name;
   }
   
   /**
    * Gets the name of this event.
    * 
    * @return the name of this event.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Sets a key-value pair of data for this event.
    * 
    * @param key the key for the data for this event.
    * @param value the value for the data for this event.
    */
   public void setData(Object key, Object value)
   {
      mDataMap.put(key, value);
   }
   
   /**
    * Gets a data value for a key for this event.
    * 
    * @param key the key for the data for this event.
    * 
    * @return the data value for the passed key for this event.
    */
   public Object getData(Object key)
   {
      return mDataMap.get(key);
   }
   
   /**
    * Sets a key-value pair of data for this event.
    * 
    * @param key the key for the data for this event.
    * @param value the value for the data for this event.
    */
   public void setData(Object key, byte value)
   {
      mDataMap.put(key, value);
   }
   
   /**
    * Gets a data value for a key for this event.
    * 
    * @param key the key for the data for this event.
    * 
    * @return the data value for the passed key for this event.
    */
   public byte getDataByteValue(Object key)
   {
      return mDataMap.getByte(key);
   }
   
   /**
    * Sets a key-value pair of data for this event.
    * 
    * @param key the key for the data for this event.
    * @param value the value for the data for this event.
    */
   public void setData(Object key, short value)
   {
      mDataMap.put(key, value);
   }
   
   /**
    * Gets a data value for a key for this event.
    * 
    * @param key the key for the data for this event.
    * 
    * @return the data value for the passed key for this event.
    */
   public short getDataShortValue(Object key)
   {
      return mDataMap.getShort(key);
   }
   
   /**
    * Sets a key-value pair of data for this event.
    * 
    * @param key the key for the data for this event.
    * @param value the value for the data for this event.
    */
   public void setData(Object key, int value)
   {
      mDataMap.put(key, value);
   }
   
   /**
    * Gets a data value for a key for this event.
    * 
    * @param key the key for the data for this event.
    * 
    * @return the data value for the passed key for this event.
    */
   public int getDataIntValue(Object key)
   {
      return mDataMap.getInt(key);
   }
   
   /**
    * Sets a key-value pair of data for this event.
    * 
    * @param key the key for the data for this event.
    * @param value the value for the data for this event.
    */
   public void setData(Object key, long value)
   {
      mDataMap.put(key, value);
   }
   
   /**
    * Gets a data value for a key for this event.
    * 
    * @param key the key for the data for this event.
    * 
    * @return the data value for the passed key for this event.
    */
   public long getDataLongValue(Object key)
   {
      return mDataMap.getLong(key);
   }
   
   /**
    * Sets a key-value pair of data for this event.
    * 
    * @param key the key for the data for this event.
    * @param value the value for the data for this event.
    */
   public void setData(Object key, float value)
   {
      mDataMap.put(key, value);
   }
   
   /**
    * Gets a data value for a key for this event.
    * 
    * @param key the key for the data for this event.
    * 
    * @return the data value for the passed key for this event.
    */
   public float getDataFloatValue(Object key)
   {
      return mDataMap.getFloat(key);
   }
   
   /**
    * Sets a key-value pair of data for this event.
    * 
    * @param key the key for the data for this event.
    * @param value the value for the data for this event.
    */
   public void setData(Object key, double value)
   {
      mDataMap.put(key, value);
   }
   
   /**
    * Gets a data value for a key for this event.
    * 
    * @param key the key for the data for this event.
    * 
    * @return the data value for the passed key for this event.
    */
   public double getDataDoubleValue(Object key)
   {
      return mDataMap.getDouble(key);
   }
   
   /**
    * Sets a key-value pair of data for this event.
    * 
    * @param key the key for the data for this event.
    * @param value the value for the data for this event.
    */
   public void setData(Object key, boolean value)
   {
      mDataMap.put(key, value);
   }
   
   /**
    * Gets a data value for a key for this event.
    * 
    * @param key the key for the data for this event.
    * 
    * @return the data value for the passed key for this event.
    */
   public boolean getDataBooleanValue(Object key)
   {
      return mDataMap.getBoolean(key);
   }
   
   /**
    * Sets a key-value pair of data for this event.
    * 
    * @param key the key for the data for this event.
    * @param value the value for the data for this event.
    */
   public void setData(Object key, char value)
   {
      mDataMap.put(key, value);
   }
   
   /**
    * Gets a data value for a key for this event.
    * 
    * @param key the key for the data for this event.
    * 
    * @return the data value for the passed key for this event.
    */
   public char getDataCharValue(Object key)
   {
      return mDataMap.getChar(key);
   }
   
   /**
    * Gets a data value for a key for this event.
    * 
    * @param key the key for the data for this event.
    * 
    * @return the data value for the passed key for this event.
    */
   public String getDataStringValue(Object key)
   {
      return mDataMap.getString(key);
   }
   
   /**
    * Copies the data from the passed event object into this one.
    * 
    * @param event the event to copy data from.
    */
   public void copyDataFrom(EventObject event)
   {
      // copy data map
      mDataMap.copyFrom(event.mDataMap);
   }
   
   /**
    * Makes a copy of this event object.
    * 
    * @return a copy of this event object.
    */
   public EventObject copy()
   {
      // copy name
      EventObject copy = new EventObject(getName());
      
      // copy data map
      copy.mDataMap = mDataMap.copy();
      
      // return copy
      return copy;
   }
}
