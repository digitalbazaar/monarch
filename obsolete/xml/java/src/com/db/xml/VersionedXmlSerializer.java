/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

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
   @Override
   public abstract String getRootTag();
   
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
      XmlElement element = new XmlElement();
      element.setParent(parent);
      
      // set name
      element.setName(getRootTag());

      // add version attribute
      element.addAttribute("version", getVersion());
      
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
      // convert version element
      setVersion(element.getAttributeValue("version"));
   }
   
   /**
    * Gets the logger for this xml serializer.
    * 
    * @return the logger for this xml serializer.
    */
   @Override
   public abstract Logger getLogger();
}
