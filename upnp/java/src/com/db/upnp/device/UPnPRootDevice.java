/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.device;

import java.net.MalformedURLException;
import java.net.URL;

import com.db.net.http.HttpWebClient;

/**
 * A UPnPRootDevice is a root UPnP device that has a UPnP server that allows it
 * to make itself available to a UPnPControlPoint. It has a "search target"
 * that identifies the type of device which allows it to be searched for. It
 * also has a location URL that points to its UPnPDeviceDescription.  
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
    * The search target for the device.
    */
   protected String mSearchTarget;

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
      setSearchTarget("upnp:rootdevice");
      setLocation("");
      setUsn("");
      
      // set no device description
      setDescription(null);
   }
   
   /**
    * Retrieves the UPnPDeviceDescription for this device from its location.
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
   public boolean retrieveDescription()
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
    * Sets the search target for this device.
    * 
    * @param searchTarget the search target (a URI) for this device.
    */
   public void setSearchTarget(String searchTarget)
   {
      mSearchTarget = searchTarget;
   }
   
   /**
    * Gets the search target for this device.
    * 
    * @return the search target (a URI) for this device.
    */
   public String getSearchTarget()
   {
      return mSearchTarget;
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
      sb.append("\nsearchTarget=" + getSearchTarget());
      sb.append("\nlocation=" + getLocation());
      sb.append("\nusn=" + getUsn());
      
      sb.append("\n");
      
      return sb.toString();
   }   
}
