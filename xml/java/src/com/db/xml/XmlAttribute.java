/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

/**
 * An XmlAttribute is an attribute for an XmlElement. It has a name and value.
 * 
 * @author Dave Longley
 */
public class XmlAttribute
{
   /**
    * The name for this XmlAttribute.
    */
   protected String mName;
   
   /**
    * The value for this XmlAttribute.
    */
   protected String mValue;
   
   /**
    * Creates a new blank XmlAttribute.
    */
   public XmlAttribute()
   {
      this("", "");
   }
   
   /**
    * Creates a new XmlAttribute with the specified name and value.
    * 
    * @param name the name of the attribute.
    * @param value the value of the attribute.
    */
   public XmlAttribute(String name, String value)
   {
      setName(name);
      setValue(value);
   }
   
   /**
    * Sets the name for this attribute.
    * 
    * @param name the name for this attribute.
    */
   public void setName(String name)
   {
      mName = name;
   }
   
   /**
    * Gets the name for this attribute.
    * 
    * @return the name for this attribute.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Sets the value for this attribute.
    * 
    * @param value the value for this attribute.
    */
   public void setValue(String value)
   {
      if(value == null)
      {
         value = "";
      }
      
      mValue = value;
   }
   
   /**
    * Gets the value for this attribute.
    * 
    * @return the value for this attribute.
    */
   public String getValue()
   {
      return mValue;
   }
   
   /**
    * Determines if this attribute is equal to another attribute.
    * 
    * @param obj the attribute to check for equality against.
    * 
    * @return true if this attribute is equal to another attribute,
    *         false if not.
    */
   public boolean equals(Object obj)
   {
      boolean rval = false;
      
      if(this != obj)
      {
         if(obj instanceof XmlAttribute && obj != null)
         {
            XmlAttribute attribute = (XmlAttribute)obj;
            String name = attribute.getName();
            String value = attribute.getValue();
            
            // check name and value
            if((getName() == null && name == null) ||
               (getName() != null && getName().equals(name)))
            {
               if((getValue() == null && value == null) ||
                  (getValue() != null && getValue().equals(value)))
               {
                  rval = true;
               }
            }
         }
      }
      else
      {
         rval = true;
      }
      
      return rval;
   }
}
