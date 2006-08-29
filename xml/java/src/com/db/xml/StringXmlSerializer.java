/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A StringXMLSerializer is a very simplistic XML serializer. Its purpose
 * is to serialize a string to XML for use with other XML objects.
 * 
 * @author Dave Longley
 */
public class StringXmlSerializer extends AbstractXmlSerializer
{
   /**
    * The string.
    */
   protected String mString;
   
   /**
    * Creates a new string xml serializer.
    */
   public StringXmlSerializer()
   {
      setString("");
   }

   /**
    * Creates a new string xml serializer.
    * 
    * @param string the string to set.
    */
   public StringXmlSerializer(String string)
   {
      setString(string);
   }
   
   /**
    * Sets the string.
    * 
    * @param string the string to set.
    */
   public void setString(String string)
   {
      mString = string;
   }
   
   /**
    * Gets the string.
    * 
    * @return the string.
    */
   public String getString()
   {
      return mString;
   }

   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "string";
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
      
      // set data
      element.setData(getString());
      
      // return element
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
      
      // get data
      setString(element.getData());
      
      rval = true;
      
      return rval;
   }

   /**
    * Gets the logger for this xml serializer.
    * 
    * @return the logger for this xml serializer.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbxml");
   }
}
