/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import java.net.MalformedURLException;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.http.HttpWebClient;
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
 * Required. URL for service description (see Service Control Protocol
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
    * The service type for this service (a URI).
    */
   protected String mServiceType;
   
   /**
    * The service ID for this service (a URI).
    */
   protected String mServiceId;
   
   /**
    * The URL to the service description (Service Protocol Control Definition)
    * for this service. It may be a relative URL.
    */
   protected String mScpdUrl;
   
   /**
    * The URL used to control this service. This is the URL that control
    * points post SOAP operations to in order to make use of this service. It
    * may be a relative URL.
    */
   protected String mControlUrl;
   
   /**
    * The URL used to handle events for this service. It may be a relative URL.
    */
   protected String mEventUrl;
   
   /**
    * The UPnPServiceDescription for this UPnPService.
    */
   protected UPnPServiceDescription mDescription;
   
   /**
    * Creates a new UPnPService with no set UPnPServiceDescription.
    */
   public UPnPService()
   {
      // set defaults
      setServiceType("urn:schemas-upnp-org:service:serviceType:v");
      setServiceId("urn:upnp-org:serviceId:serviceID");
      setScpdUrl("");
      setControlUrl("");
      setEventUrl("");
      
      // set the description for this service to null
      setDescription(null);
   }
   
   /**
    * Retrieves the UPnPServiceDescription for this service from its SCPD URL.
    * 
    * This method will do an HTTP GET to retrieve the UPnP Service Control
    * Protocol Description from the URL set by setSpcdUrl().
    * 
    * @return true if the service description was retrieved successfully,
    *         false if not.
    * 
    * @exception MalformedURLException thrown if the URL from getScpdUrl()
    *                                  is malformed.
    */
   public boolean retrieveDescription()
   throws MalformedURLException
   {
      boolean rval = false;
      
      // create http client and get the xml from the location 
      HttpWebClient client = new HttpWebClient();
      String xml = client.getContent(getScpdUrl());
      if(xml != null)
      {
         // create a new UPnPServiceDescription
         UPnPServiceDescription description = new UPnPServiceDescription();
         
         // convert the description from the retrieved xml
         if(description.convertFromXml(xml))
         {
            // set the description to this service
            setDescription(description);
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "service";
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
      XmlElement serviceElement = new XmlElement(getRootTag());
      serviceElement.setParent(parent);
      
      // create service type element
      XmlElement serviceTypeElement = new XmlElement("serviceType");
      serviceTypeElement.setValue(getServiceType());
      serviceElement.addChild(serviceTypeElement);
      
      // create service ID element
      XmlElement serviceIdElement = new XmlElement("serviceId");
      serviceIdElement.setValue(getServiceId());
      serviceElement.addChild(serviceIdElement);
      
      // create scpd URL element
      XmlElement scpdUrlElement = new XmlElement("SCPDURL");
      scpdUrlElement.setValue(getScpdUrl());
      serviceElement.addChild(scpdUrlElement);
      
      // create control URL element
      XmlElement controlUrlElement = new XmlElement("controlURL");
      controlUrlElement.setValue(getControlUrl());
      serviceElement.addChild(controlUrlElement);
      
      // create event URL element
      XmlElement eventUrlElement = new XmlElement("eventSubURL");
      eventUrlElement.setValue(getEventUrl());
      serviceElement.addChild(eventUrlElement);
      
      // return root element
      return serviceElement;
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
      
      // get service type element
      setServiceType(element.getFirstChildValue("serviceType"));
      
      // get service ID element
      setServiceId(element.getFirstChildValue("serviceId"));
      
      // get scpd URL element
      setScpdUrl(element.getFirstChildValue("SCPDURL"));
      
      // get control URL element
      setControlUrl(element.getFirstChildValue("controlURL"));
      
      // get event URL element
      setEventUrl(element.getFirstChildValue("eventSubURL"));
      
      return rval;
   }
   
   /**
    * Sets the service type for this service (a URI).
    * 
    * @param serviceType the service type for this service.
    */
   public void setServiceType(String serviceType)
   {
      mServiceType = serviceType;
   }
   
   /**
    * Gets the service type for this service (a URI).
    * 
    * @return the service type for this service.
    */
   public String getServiceType()
   {
      return mServiceType;
   }
   
   /**
    * Sets the service ID for this service (a URI).
    * 
    * @param serviceId the service ID for this service.
    */
   public void setServiceId(String serviceId)
   {
      mServiceId = serviceId;
   }
   
   /**
    * Gets the service ID for this service (a URI).
    * 
    * @return the service ID for this service.
    */
   public String getServiceId()
   {
      return mServiceId;
   }

   /**
    * Sets the URL to the service description (Service Protocol Control
    * Definition) for this service. It may be a relative URL.
    * 
    * @param scpdUrl the URL to the service description for this service.
    */
   public void setScpdUrl(String scpdUrl)
   {
      mScpdUrl = scpdUrl;
   }
   
   /**
    * Gets the URL to the service description (Service Protocol Control
    * Definition) for this service. It may be a relative URL.
    * 
    * @return the URL to the service description for this service.
    */
   public String getScpdUrl()
   {
      return mScpdUrl;
   }
   
   /**
    * Sets the URL used to control this service. This is the URL that control
    * points post SOAP operations to in order to make use of this service. It
    * may be a relative URL.
    * 
    * @param controlUrl the URL to control this service.
    */
   public void setControlUrl(String controlUrl)
   {
      mControlUrl = controlUrl;
   }
   
   /**
    * Gets the URL used to control this service. This is the URL that control
    * points post SOAP operations to in order to make use of this service. It
    * may be a relative URL.
    * 
    * @return the URL to control this service.
    */
   public String getControlUrl()
   {
      return mControlUrl;
   }
   
   /**
    * Sets the URL used to handle events for this service. It may be a relative
    * URL. May be a blank string if there are no events for this service.
    * 
    * @param eventUrl the URL for events for this service (may be a blank
    *                 string if there are no events for this service).
    */
   public void setEventUrl(String eventUrl)
   {
      mEventUrl = eventUrl;
   }
   
   /**
    * Gets the URL used to handle events for this service. It may be a relative
    * URL. May be a blank string if there are no events for this service.
    * 
    * @return the URL for events for this service (may be a blank string if
    *         there are no events for this service).
    */
   public String getEventUrl()
   {
      return mEventUrl;
   }
   
   /**
    * Sets the UPnPServiceDescription for this UPnPService.
    * 
    * @param description the UPnPServiceDescription for this UPnPService.
    */
   public void setDescription(UPnPServiceDescription description)
   {
      mDescription = description;
   }

   /**
    * Gets the UPnPServiceDescription for this UPnPService. This method
    * will return null if no description has been set for this UPnPService.
    * 
    * @return the UPnPServiceDescription for this UPnPService (can be null).
    */
   public UPnPServiceDescription getDescription()   
   {
      return mDescription;
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
