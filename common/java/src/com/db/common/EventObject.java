/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import java.util.HashMap;

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
   protected HashMap mDataMap;
   
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
      mDataMap = new HashMap();
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
    * @return the data value for the passed key for this event.
    */
   public Object getData(Object key)
   {
      return mDataMap.get(key);
   }
}
