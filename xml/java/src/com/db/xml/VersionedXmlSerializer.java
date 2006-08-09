/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

import org.w3c.dom.Element;

import com.db.logging.Logger;

/**
 * A VersionedXmlSerializer is a class that implements IXmlSerializer
 * and has a version.
 * 
 * @author Dave Longley
 */
public abstract class VersionedXmlSerializer extends AbstractXmlSerializer
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
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public abstract String getRootTag();
   
   /**
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    *
    * @param indentLevel the number of spaces to place before the text
    *                    after each new line.
    *                    
    * @return the XML-based representation of the object.
    */
   public abstract String convertToXml(int indentLevel);

   /**
    * This method takes a parsed DOM XML element and converts it
    * back into this object's representation.
    *
    * @param element the parsed element that contains this objects information.
    * 
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
