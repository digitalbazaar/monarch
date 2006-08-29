/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.wsdl;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;
   
/**
 * A WSDL message part. A message part is like a parameter (for an input
 * message) or a return value (for an output message).
 * 
 * The part has a type associated with it.
 * 
 * @author Dave Longley
 */
public class WsdlMessagePart extends AbstractXmlSerializer
{
   /**
    * The name of this part.
    */
   protected String mName;
   
   /**
    * The type of this part.
    */
   protected String mType;

   /**
    * Creates a new blank WsdlMessagePart. 
    */
   public WsdlMessagePart()
   {
      this("", "");
   }
   
   /**
    * Creates a new WsdlMessagePart. 
    * 
    * @param name the name of the part.
    * @param type the type of the part.
    */
   public WsdlMessagePart(String name, String type)
   {
      setName(name);
      setType(type);
   }
   
   /**
    * Sets this part's name.
    * 
    * @param name the name of this part. 
    */
   public void setName(String name)
   {
      mName = name;
   }
   
   /**
    * Gets this part's name.
    * 
    * @return the name of this part. 
    */
   public String getName()
   {
      return mName;
   }      
   
   /**
    * Sets the type of this part.
    * 
    * @param type the type of this part.
    */
   public void setType(String type)
   {
      mType = type;
   }
   
   /**
    * Gets the type of this part.
    * 
    * @return the type of this part.
    */
   public String getType()
   {
      return mType;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "part";
   }
   
   /**
    * Creates an XmlElement from this object.
    *
    * @return the XmlElement that represents this object.
    */
   public XmlElement convertToXmlElement()
   {
      // create xml element
      XmlElement element = new XmlElement(getRootTag());
      
      // add attributes
      element.addAttribute("name", getName());
      element.addAttribute("type", getType());
      
      return element;      
   }
   
   /**
    * Converts this object from an XmlElement.
    *
    * @param element the XmlElement to convert from.
    * 
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXmlElement(XmlElement element)   
   {
      boolean rval = false;
      
      // clear part name and part type
      setName("");
      setType("");
      
      if(element.getName().equals(getRootTag()))
      {
         // get part name
         setName(element.getAttributeValue("name"));
         
         // get part type
         setType(element.getAttributeValue("type"));
         
         // ensure there is a part name and type
         if(!getName().equals("") && !getType().equals(""))            
         {
            // conversion successful
            rval = true;
         }
      }
      
      return rval;
   }   
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }      
}
