/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

import org.w3c.dom.Element;

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
    * This method takes XML text (in full document form) and converts
    * it to it's internal representation.
    *
    * @param indentLevel the number of spaces to place before the text
    *                    after each new line.
    * @return true if successful, false otherwise.    
    */
   public String convertToXml(int indentLevel)
   {
      StringBuffer xml = new StringBuffer();
      StringBuffer indent = new StringBuffer("\n");
      for(int i = 0; i < indentLevel; i++)
      {
         indent.append(' ');
      }

      xml.append('<');
      xml.append(getRootTag());
      xml.append('>');
      xml.append(XmlCoder.encode(getString()));
      xml.append("</");
      xml.append(getRootTag());
      xml.append('>');

      return xml.toString();
   }
   
   /**
    * This method takes a parsed DOM XML element and converts it
    * back into this object's representation.
    *
    * @param element the parsed element that contains this objects information.
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXml(Element element)
   {
      boolean rval = true;
      
      ElementReader er = new ElementReader(element);
      setString(XmlCoder.decode(er.getStringValue()));
      
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
