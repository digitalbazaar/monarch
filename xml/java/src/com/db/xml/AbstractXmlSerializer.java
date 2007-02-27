/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

import java.io.StringReader;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;

import com.db.logging.Logger;
import com.db.xml.IXmlSerializer;

/**
 * An AbstractXmlSerializer is a class that implements the basic
 * IXmlSerializer methods.
 * 
 * A logger method is also provided for logging information during
 * xml serialization/deserialization.
 * 
 * @author Dave Longley
 */
public abstract class AbstractXmlSerializer implements IXmlSerializer
{
   /**
    * Creates a new AbstractXmlSerializer.
    */
   public AbstractXmlSerializer()
   {
   }

   /**
    * This method takes options that are used to configure
    * how to convert to and from xml.
    *
    * @param options the configuration options.
    * 
    * @return true if options successfully set, false if not.
    */
   public boolean setXmlSerializerOptions(int options)
   {
      // no options by default
      return false;
   }

   /**
    * This method gets the options that are used to configure
    * how to convert to and from xml.
    *
    * @return the configuration options.
    */
   public int getXmlSerializerOptions()
   {
      // no options by default
      return 0;
   }
   
   /**
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    *
    * @return the XML-based representation of the object.
    */
   public String convertToXml()
   {
      // defaults to including a header
      return convertToXml(true, 0, 1);
   }
   
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
      boolean header, int indentSize, int childIndentSize)
   {
      String rval = "";
      
      try
      {
         // convert this object to an xml element
         XmlElement element = convertToXmlElement(null);
         
         // convert the element to XML
         rval = element.convertToXml(header, indentSize, childIndentSize);
      }
      catch(Throwable t)
      {
         // print error and stack trace
         getLogger().error(getClass(), 
            "Exception thrown while converting to xml!" +
            ",\nexception= " + t +
            ",\ncause= " + t.getCause());
         getLogger().debug(getClass(), 
            "Exception thrown while converting to xml!" +
            ",\ntrace= " + Logger.getStackTrace(t));
      }

      return rval;
   }
   
   /**
    * Converts this object from XML.
    *
    * @param xmlText the xml text document that represents the object.
    * 
    * @exception XmlException thrown if this object could not be converted from
    *                         xml.
    */
   public void convertFromXml(String xmlText) throws XmlException
   {
      getLogger().detail(getClass(),
         "converting " + getClass() + " from xml...");
      
      try
      {
         // get document builder
         DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
         DocumentBuilder builder = factory.newDocumentBuilder();
         
         // read text
         InputSource is = new InputSource(new StringReader(xmlText));
         Document doc = builder.parse(is);
         
         // convert from xml
         convertFromXml(doc.getDocumentElement());
      }
      catch(Throwable t)
      {
         // print error and stack trace
         getLogger().error(getClass(), 
            "Exception thrown while converting from xml!" +
            ",\nexception= " + t +
            ",\ncause= " + t.getCause());
         getLogger().debug(getClass(), 
            "Exception thrown while converting from xml!" +
            ",\ntrace= " + Logger.getStackTrace(t));
         
         // throw new xml exception
         throw new XmlException(
            "Could not convert " + getClass() + " from xml!,cause=" + t, t);
      }
   }
   
   /**
    * This method takes a parsed DOM XML element and converts it
    * back into this object's representation.
    *
    * @param element the parsed element that contains this objects information.
    * 
    * @exception XmlException thrown if this object could not be converted from
    *                         xml.
    */
   public void convertFromXml(Element element) throws XmlException
   {
      // create an XmlElement
      XmlElement xmlElement = new XmlElement();
      
      // convert the element from the passed DOM element
      xmlElement.convertFromXml(element);
      
      // convert this object from the xml element
      convertFromXmlElement(xmlElement);
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public abstract String getRootTag();
   
   /**
    * Creates an XmlElement from this object.
    *
    * @param parent the parent XmlElement for the XmlElement being created
    *               (can be null). 
    * 
    * @return the XmlElement that represents this object.
    */
   public abstract XmlElement convertToXmlElement(XmlElement parent);
   
   /**
    * Converts this object from an XmlElement.
    *
    * @param element the XmlElement to convert from.
    * 
    * @exception XmlException thrown if this object could not be converted from
    *                         xml.
    */
   public void convertFromXmlElement(XmlElement element)
      throws XmlException
   {
      if(!element.getName().equals(getRootTag()))
      {
         throw new XmlException(
            "Invalid element name for " + getClass() + "! " +
            "Looking for \"" + getRootTag() +
            "\" but found \"" + element.getName() + "\"");
      }
   }
   
   /**
    * Gets the logger for this xml serializer.
    * 
    * @return the logger for this xml serializer.
    */
   public abstract Logger getLogger();
}
