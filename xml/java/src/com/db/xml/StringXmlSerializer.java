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
   @Override
   public String getRootTag()   
   {
      return "string";
   }
   
   /**
    * Creates an XmlElement from this object.
    *
    * @param parent the parent XmlElement for the XmlElement being created
    *               (can be null). 
    * 
    * @return the XmlElement that represents this object.
    */
   @Override
   public XmlElement convertToXmlElement(XmlElement parent)
   {
      // create xml element
      XmlElement element = new XmlElement(getRootTag());
      element.setParent(parent);
      
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
    * @exception XmlException thrown if this object could not be converted from
    *                         xml.
    */
   @Override
   public void convertFromXmlElement(XmlElement element) throws XmlException
   {
      // get data
      setString(element.getData());
   }
   
   /**
    * Gets the logger for this xml serializer.
    * 
    * @return the logger for this xml serializer.
    */
   @Override
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbxml");
   }
}
