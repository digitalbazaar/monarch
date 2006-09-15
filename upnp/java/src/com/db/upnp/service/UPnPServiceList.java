/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A UPnPServiceList represents a list of UPnPServices.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
 * -----------------------------------------------------------------------
 * <pre>
 * 
 * <serviceList>
 *   <service>
 *     <serviceType>urn:schemas-upnp-org:service:serviceType:v</serviceType>
 *     <serviceId>urn:upnp-org:serviceId:serviceID</serviceId>
 *     <SCPDURL>URL to service description</SCPDURL>
 *     <controlURL>URL for control</controlURL>
 *     <eventSubURL>URL for eventing</eventSubURL>
 *   </service>
 *   Declarations for other services defined by a UPnP Forum working
 *   committee (if any) go here
 *   Declarations for other services added by UPnP vendor (if any) go here
 * </serviceList>
 * 
 * </pre>
 * 
 * serviceList
 * Required. Contains the following sub elements:
 * 
 * service
 * Required. Repeated once for each service defined by a UPnP Forum working 
 * committee. If UPnP vendor differentiates device by adding additional,
 * standard UPnP services, repeated once for additional service.
 * -----------------------------------------------------------------------
 * 
 * @author Dave Longley
 */
public class UPnPServiceList extends AbstractXmlSerializer
{
   /**
    * Creates a new UPnPServiceList.
    */
   public UPnPServiceList()
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
      XmlElement rootElement = new XmlElement(getRootTag());
      rootElement.setParent(parent);
      
      // FIXME:
      
      // return root element
      return rootElement;
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
    * Gets the logger for this UPnPServiceList.
    * 
    * @return the logger for this UPnPServiceList.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
