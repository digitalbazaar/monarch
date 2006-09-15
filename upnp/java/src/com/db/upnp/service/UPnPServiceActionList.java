/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A UPnPServiceActionList is a XML serializable list of UPnPServiceActions.
 * 
 * @author Dave Longley
 */
public class UPnPServiceActionList extends AbstractXmlSerializer
{
   /**
    * Creates a new UPnPServiceActionList.
    */
   public UPnPServiceActionList()
   {
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "root";
   }
   
   /**
    * Creates an XmlElement from this object.
    *
    * @param parent the parent XmlElement for the XmlElement being created
    *               (can be null). 
    * 
    * @return the XmlElement that represents this object.
    */
   public XmlElement convertToXmlElement(XmlElement parent)   
   {
      // create the root element
      XmlElement scpdElement = new XmlElement(getRootTag());
      scpdElement.setParent(parent);
      
      // FIXME:
      
      // return root element
      return scpdElement;
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
      boolean rval = true;

      // FIXME:
      
      return rval;
   }
   
   /**
    * Gets the logger for this UPnPServiceActionList.
    * 
    * @return the logger for this UPnPServiceActionList.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
