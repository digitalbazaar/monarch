/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.device;

import java.net.MalformedURLException;
import java.net.URL;

import com.db.net.http.HttpWebClient;
import com.db.upnp.service.UPnPService;
import com.db.upnp.service.UPnPServiceDescription;

/**
 * A UPnPRootDevice is a root UPnP device that has a UPnP server that allows it
 * to make itself available to a UPnPControlPoint. It has a location URL that
 * points to its UPnPDeviceDescription and a Unique Service Name.  
 * 
 * @author Dave Longley
 */
public class UPnPRootDevice
{
   /**
    * The server for the device.
    */
   protected String mServer;
   
   /**
    * The location (a URL) for the device's description.
    */
   protected String mLocation;
   
   /**
    * The Unique Service Name for the device.
    */
   protected String mUsn;
   
   /**
    * The UPnPDeviceDescription for this device.
    */
   protected UPnPDeviceDescription mDescription;   
   
   /**
    * Creates a new UPnPRootDevice with no set UPnPDeviceDescription.
    */
   public UPnPRootDevice()
   {
      // set defaults
      setServer("");
      setLocation("");
      setUsn("");
      
      // set no device description
      setDescription(null);
   }
   
   /**
    * Retrieves the UPnPDeviceDescription for this device from its location.
    * The description will be set to this device if it is successfully
    * retrieved.
    * 
    * This method will do an HTTP GET to retrieve the UPnP Device Description
    * from the URL set by setLocation().
    * 
    * @return true if the device description was retrieved successfully,
    *         false if not.
    * 
    * @exception MalformedURLException thrown if the URL from getLocation()
    *                                  is malformed.
    */
   public boolean retrieveDeviceDescription()
   throws MalformedURLException
   {
      boolean rval = false;
      
      // create http client and get the xml from the location 
      HttpWebClient client = new HttpWebClient();
      URL url = new URL(getLocation());
      String xml = client.getContent(url, url.getPath());
      if(xml != null)
      {
         // create a new UPnPDeviceDescription
         UPnPDeviceDescription description = new UPnPDeviceDescription();
         
         // convert the description from the retrieved xml
         if(description.convertFromXml(xml))
         {
            // set the description to this device
            setDescription(description);
            
            // see if the description has no set base URL
            if(description.getBaseUrl().equals(""))
            {
               // set the description's base URL to the location URL
               description.setBaseUrl(getLocation());
            }
            
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Retrieves the UPnPServiceDescription for the specified service from its
    * SCPD URL. The description will be set to the passed service if it is
    * successfully retrieved.
    * 
    * This method will do an HTTP GET to retrieve the UPnP Service Control
    * Protocol Description from the URL set by setSpcdUrl().
    * 
    * This method will fail if the description for this device is not yet
    * available. Make sure to set it or retrieve it via the method
    * retrieveDeviceDescription() before calling this method.
    * 
    * @param service the service to retrieve the description of.
    * 
    * @return true if the service description was retrieved successfully,
    *         false if not.
    * 
    * @exception MalformedURLException thrown if the URL from getScpdUrl()
    *                                  is malformed.
    */
   public boolean retrieveServiceDescription(UPnPService service)
   throws MalformedURLException
   {
      boolean rval = false;
      
      if(getDescription() != null)
      {
         // create http client and get the xml from the location 
         HttpWebClient client = new HttpWebClient();
         URL url = new URL(
            getDescription().getBaseUrl() + service.getScpdUrl());
         String xml = client.getContent(url, url.getPath());
         if(xml != null)
         {
            // create a new UPnPServiceDescription
            UPnPServiceDescription description = new UPnPServiceDescription();
            
            // convert the description from the retrieved xml
            if(description.convertFromXml(xml))
            {
               // set the description to the service
               service.setDescription(description);
               rval = true;
            }
         }
      }
      
      return rval;
   }   
   
   /**
    * Sets the server for this device.
    * 
    * @param server the server for this device.
    */
   public void setServer(String server)
   {
      mServer = server;
   }
   
   /**
    * Gets the server for this device.
    * 
    * @return the server for this device.
    */
   public String getServer()
   {
      return mServer;
   }
   
   /**
    * Sets the location (a URL) for this device's description.
    * 
    * @param location the location for this device's description.
    */
   public void setLocation(String location)   
   {
      mLocation = location;
   }
   
   /**
    * Gets the location (a URL) for this device's description.
    * 
    * @return the location for this device's description.
    */
   public String getLocation()
   {
      return mLocation;
   }
   
   /**
    * Sets the Unique Service Name for this device.
    * 
    * @param usn the Unique Service Name for this device.
    */
   public void setUsn(String usn)
   {
      mUsn = usn;
   }
   
   /**
    * Gets the Unique Service Name for this device.
    * 
    * @return the Unique Service Name for this device.
    */
   public String getUsn()
   {
      return mUsn;
   }
   
   /**
    * Sets the UPnPDeviceDescription for this UPnPDevice.
    * 
    * @param description the UPnPDeviceDescription for this UPnPDevice.
    */
   public void setDescription(UPnPDeviceDescription description)
   {
      mDescription = description;
   }

   /**
    * Gets the UPnPDeviceDescription for this UPnPDevice. This method
    * will return null if no description has been set for this UPnPDevice.
    * 
    * @return the UPnPDeviceDescription for this UPnPDevice (can be null).
    */
   public UPnPDeviceDescription getDescription()   
   {
      return mDescription;
   }   
   
   /**
    * Gets the string representation for this UPnPRootDevice.
    * 
    * @return the string representation for this UPnPRootDevice.
    */
   public String toString()
   {
      StringBuffer sb = new StringBuffer();
      
      sb.append("[UPnPRootDevice]");
      
      sb.append("\nserver=" + getServer());
      sb.append("\nlocation=" + getLocation());
      sb.append("\nusn=" + getUsn());
      
      sb.append("\n");
      
      return sb.toString();
   }   
}
