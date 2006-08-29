/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
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
   public abstract String getRootTag();
   
   /**
    * Creates an XmlElement from this object.
    *
    * @return the XmlElement that represents this object.
    */
   public XmlElement convertToXmlElement()
   {
      // create xml element
      XmlElement element = new XmlElement();
      
      // set name
      element.setName(getRootTag());

      // add version attribute
      element.getAttributeMap().addAttribute("version", getVersion());
      
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

      if(element != null && element.getAttributeMap().hasAttribute("version"))
      {
         // convert version element
         setVersion(element.getAttributeMap().getAttributeValue("version"));
         
         // version attribute exists
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets the logger for this xml serializer.
    * 
    * @return the logger for this xml serializer.
    */
   public abstract Logger getLogger();
}
