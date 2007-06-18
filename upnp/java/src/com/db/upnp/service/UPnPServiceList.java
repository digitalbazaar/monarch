/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;
import com.db.xml.XmlException;

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
implements Iterable<UPnPService>
{
   /**
    * The UPnPServices for this list.
    */
   protected List<UPnPService> mServices;
   
   /**
    * Creates a new UPnPServiceList.
    */
   public UPnPServiceList()
   {
      // create the services list
      mServices = new LinkedList<UPnPService>();
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   @Override
   public String getRootTag()   
   {
      return "serviceList";
   }
   
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
      // create the root element
      XmlElement listElement = new XmlElement(getRootTag());
      listElement.setParent(parent);
      
      // convert each service to an xml element child
      for(UPnPService service: getServices())
      {
         listElement.addChild(service.convertToXmlElement(listElement));
      }
      
      // return root element
      return listElement;
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
      super.convertFromXmlElement(element);
      
      // clear service list
      clear();
      
      // convert services
      for(XmlElement serviceElement: element.getChildren("service"))
      {
         UPnPService service = new UPnPService();
         service.convertFromXmlElement(serviceElement);
         addService(service);
      }
   }
   
   /**
    * Adds a UPnPService to this list.
    * 
    * @param service the UPnPService to add.
    */
   public void addService(UPnPService service)
   {
      getServices().add(service);
   }
   
   /**
    * Removes a UPnPService from this list.
    * 
    * @param service the UPnPService to remove.
    */
   public void removeService(UPnPService service)
   {
      getServices().remove(service);
   }
   
   /**
    * Gets a particular service from this list according to its service ID.
    * 
    * @param serviceId the service ID of the UPnPService to retrieve. 
    * 
    * @return the retrieved UPnPService or null if no match was found.
    */
   public UPnPService getService(String serviceId)
   {
      UPnPService rval = null;
      
      for(Iterator<UPnPService> i = iterator(); i.hasNext() && rval == null;)
      {
         UPnPService service = i.next();
         if(service.getServiceId().equals(serviceId))
         {
            rval = service;
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the first service from this list with the specified service type.
    * 
    * @param serviceType the service type of the UPnPService to retrieve. 
    * 
    * @return the retrieved UPnPService or null if no match was found.
    */
   public UPnPService getFirstService(String serviceType)
   {
      UPnPService rval = null;
      
      for(Iterator<UPnPService> i = iterator(); i.hasNext() && rval == null;)
      {
         UPnPService service = i.next();
         if(service.getServiceType().equals(serviceType))
         {
            rval = service;
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the UPnPService for this list in a vector.
    * 
    * @return the UPnPService for this list in a vector.
    */
   public List<UPnPService> getServices()
   {
      return mServices;
   }
   
   /**
    * Clears the services from this list.
    */
   public void clear()
   {
      getServices().clear();
   }
   
   /**
    * Gets an iterator over the services in this list.
    *
    * @return an iterator over the UPnPServices in this list.
    */
   public Iterator<UPnPService> iterator()
   {
      return getServices().iterator();
   }
   
   /**
    * Gets the number of services in this list.
    * 
    * @return the number of services in this list.
    */
   public int getServiceCount()
   {
      return getServices().size();
   }
   
   /**
    * Gets the logger for this UPnPServiceList.
    * 
    * @return the logger for this UPnPServiceList.
    */
   @Override
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
