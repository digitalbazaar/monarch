/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

/**
 * An interface that allows an object to be serialized to
 * and from XML.
 * 
 * FUTURE CODE: This interface is a little clunky and doesn't take advantage
 * of any good xml parsing technology. We want to improve it in the future.
 *  
 * @author Dave Longley
 * @author Manu Sporny
 */
public interface IXmlSerializer
{
   /**
    * This method takes options that are used to configure
    * how to convert to and from xml.
    *
    * @param options the configuration options.
    * 
    * @return true if options successfully set, false if not.
    */
   public boolean setXmlSerializerOptions(int options);

   /**
    * This method gets the options that are used to configure
    * how to convert to and from xml.
    *
    * @return the configuration options.
    */
   public int getXmlSerializerOptions();
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag();
   
   /**
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    *
    * @return the XML-based representation of the object.
    */
   public String convertToXml();
   
   /**
    * Converts this object to XML.
    * 
    * @param header true to include an XML header, false not to.
    * @param indentSize the number of spaces to indent this element.
    * @param childIndentSize the number of additional spaces to indent
    *                        each child.
    * 
    * @return the XML for this object.
    */
   public String convertToXml(
      boolean header, int indentSize, int childIndentSize);
   
   /**
    * Creates an XmlElement from this object.
    *
    * @param parent the parent XmlElement for the XmlElement being created
    *               (can be null). 
    * 
    * @return the XmlElement that represents this object.
    */
   public XmlElement convertToXmlElement(XmlElement parent);
   
   /**
    * This method takes XML text (in full document form) and converts
    * it to its internal representation.
    *
    * @param xmlText the xml text document that represents the object.
    * 
    * @return true if successful, false otherwise.    
    */
   public boolean convertFromXml(String xmlText);
   
   /**
    * Converts this object from an XmlElement.
    *
    * @param element the XmlElement to convert from.
    * 
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXmlElement(XmlElement element);
}
