/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A UPnPService represents a service that supports the UPnP protocol. A
 * UPnPService is provided by a UPnPDevice to give a UPnPControlPoint access
 * to its functionality.
 *
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
 * -----------------------------------------------------------------------
 * 
 * <pre>
 * 
 * <service>
 *   <serviceType>urn:schemas-upnp-org:service:serviceType:v</serviceType>
 *   <serviceId>urn:upnp-org:serviceId:serviceID</serviceId>
 *   <SCPDURL>URL to service description</SCPDURL>
 *   <controlURL>URL for control</controlURL>
 *   <eventSubURL>URL for eventing</eventSubURL>
 * </service>
 *     
 * </pre>
 * 
 * service
 * Required. Repeated once for each service defined by a UPnP Forum working 
 * committee. If UPnP vendor differentiates device by adding additional,
 * standard UPnP services, repeated once for additional service.
 * Contains the following sub elements:
 * 
 * serviceType
 * Required. UPnP service type. Must not contain a hash character (#, 23 Hex
 * in UTF-8).
 * 
 * For standard service types defined by a UPnP Forum working committee, must
 * begin with urn:schemas-upnp-org:service: followed by a service type suffix,
 * colon, and an integer service version (as shown in the listing above).
 * For non-standard service types specified by UPnP vendors, must begin with
 * urn:, followed by an ICANN domain name owned by the vendor, followed by
 * :service:, followed by a service type suffix, colon, and an integer service
 * version, i.e., urn:domain-name:service:serviceType:v.
 * The service type suffix defined by a UPnP Forum working committee or
 * specified by a UPnP vendor must be <= 64 characters, not counting the
 * version suffix and separating colon. Single URI.
 * 
 * serviceId
 * Required. Service identifier. Must be unique within this device description.
 * For standard services defined by a UPnP Forum working committee, must begin
 * with urn:upnp-org:serviceId: followed by a service ID suffix (as shown in
 * the listing above). (Note that upnp-org is used instead of schemas-upnp-org
 * in this case because an XML schema is not defined for each service ID.)
 * For non-standard services specified by UPnP vendors, must begin with urn:,
 * followed by an ICANN domain name owned by the vendor, followed by
 * :serviceId:, followed by a service ID suffix, i.e.,
 * urn:domain-name:serviceId:serviceID.
 * The service ID suffix defined by a UPnP Forum working committee or
 * specified by a UPnP vendor must be <= 64 characters. Single URI.
 * 
 * SCPDURL
 * Required. URL for service description (nee Service Control Protocol
 * Definition URL). (cf. section below on service description.) May be relative
 * to base URL. Specified by UPnP vendor. Single URL.
 * 
 * controlURL
 * Required. URL for control (cf. section on Control). May be relative to base
 * URL. Specified by UPnP vendor. Single URL.
 * 
 * eventSubURL
 * Required. URL for eventing (cf. section on Eventing). May be relative to
 * base URL. Must be unique within the device; no two services may have the
 * same URL for eventing. If the service has no evented variables, it should
 * not have eventing (cf. section on Eventing); if the service does not have
 * eventing, this element must be present but should be empty, i.e.,
 * <eventSubURL></eventSubURL>. Specified by UPnP vendor. Single URL.
 * -----------------------------------------------------------------------
 * 
 * @author Dave Longley
 */
public class UPnPService extends AbstractXmlSerializer
{
   /**
    * Creates a new UPnPService.
    */
   public UPnPService()
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
    * Gets the logger for this UPnPService.
    * 
    * @return the logger for this UPnPService.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
