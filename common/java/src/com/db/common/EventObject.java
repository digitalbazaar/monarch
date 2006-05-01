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
    * The data bundled with this event.
    */
   protected Object mData;
   
   /**
    * Creates a new EventObject with no specified name and null data.
    */
   public EventObject()
   {
      this("", null);
   }

   /**
    * Creates a new EventObject with the given name and null data.
    * 
    * @param name the name of this event.
    */
   public EventObject(String name)
   {
      this(name, null);
   }

   /**
    * Creates a new EventObject with the given name and data.
    * 
    * @param name the name of this event.
    * @param data the data for this event.
    */
   public EventObject(String name, Object data)
   {
      setName(name);
      setData(data);
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
    * Sets the data for this event.
    * 
    * @param data the data for this event.
    */
   public void setData(Object data)
   {
      mData = data;
   }
   
   /**
    * Gets the data for this event.
    * 
    * @return the data for this event.
    */
   public Object getData()
   {
      return mData;
   }
}
