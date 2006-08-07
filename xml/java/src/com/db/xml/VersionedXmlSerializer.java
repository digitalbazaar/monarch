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
 * A VersionedXmlSerializer is a class that implements IXmlSerializer
 * and has a version.
 * 
 * @author Dave Longley
 */
public abstract class VersionedXmlSerializer implements IXmlSerializer
{
   /**
    * The version of this xml serializer.
    */
   protected String mVersion;
   
   /**
    * Creates a new VersionedXmlSerializer.
    * 
    * @param version the version for this xml serializer.
    */
   public VersionedXmlSerializer(String version)
   {
      // set version
      setVersion(version);
   }

   /**
    * Sets the version of this xml serializer.
    * 
    * @param version the version.
    */
   public void setVersion(String version)
   {
      mVersion = version;
   }

   /**
    * Gets the version of this xml serializer.
    * 
    * @return the version.
    */
   public String getVersion()
   {
      return mVersion;
   }
   
   /**
    * This method takes options that are used to configure
    * how to convert to and from xml.
    *
    * @param options the configuration options.
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
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public abstract String getRootTag();
   
   /**
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    *
    * @return the XML-based representation of the object.
    */
   public String convertToXml()
   {
      String rval = "";
      
      try
      {
         rval = convertToXml(0);
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
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    *
    * @param indentLevel the number of spaces to place before the text
    *                    after each new line.
    * @return the XML-based representation of the object.
    */
   public abstract String convertToXml(int indentLevel);

   /**
    * This method takes XML text (in full document form) and converts
    * it to its internal representation.
    *
    * @param xmlText the xml text document that represents the object.
    * @return true if successful, false otherwise.    
    */
   public boolean convertFromXml(String xmlText)
   {
      boolean rval = false;
      
      getLogger().detail(getClass(), "converting from xml...");
     
      try
      {
         // get document builder
         DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
         DocumentBuilder builder = factory.newDocumentBuilder();
         
         // read text
         InputSource is = new InputSource(new StringReader(xmlText));
         Document doc = builder.parse(is);
         
         // normalize text representation
         doc.getDocumentElement().normalize();
         
         // convert from xml
         if(convertFromXml(doc.getDocumentElement()))
         {
            getLogger().detail(getClass(),
               "successfully converted from xml.");
            
            rval = true;
         }
         else
         {
            getLogger().detail(getClass(), 
               "failed to convert from xml.");
         }
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
      }

      return rval;
   }
   
   /**
    * This method takes a parsed DOM XML element and converts it
    * back into this object's representation.
    *
    * @param element the parsed element that contains this objects information.
    * @return true if successful, false otherwise.
    */
   public abstract boolean convertFromXml(Element element);
   
   /**
    * Gets the logger for this xml serializer.
    * 
    * @return the logger for this xml serializer.
    */
   public abstract Logger getLogger();
}
