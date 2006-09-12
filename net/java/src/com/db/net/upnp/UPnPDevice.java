/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.upnp;

/**
 * A UPnP Device. This object represents a device that supports the
 * UPnP protocol. It can be controlled via a UPnPControlPoint.
 * 
 * @author Dave Longley
 */
public class UPnPDevice
{
   /**
    * The USN for the device.
    */
   protected String mUsn;
   
   /**
    * The service type for this device.
    */
   protected String mServiceType;
   
   /**
    * The server for this device.
    */
   protected String mServer;
   
   /**
    * The location for the device's UPnP interface description.
    */
   protected String mLocation;
   
   /**
    * Creates a new UPnPDevice from a SsdpDiscoverResponse.
    * 
    * @param response the SsdpDiscoverResponse to create this device from.
    */
   public UPnPDevice(SsdpDiscoverResponse response)
   {
      // get device information
      mUsn = response.getUsn();
      mServiceType = response.getServiceType();
      mServer = response.getServer();
      mLocation = response.getLocation();
   }

   /**
    * Gets the usn for the device.
    * 
    * @return the usn for the device.
    */
   public String getUsn()
   {
      return mUsn;
   }
   
   /**
    * Gets the service type for this device
    * 
    * @return the service type (a URI) for this device.
    */
   public String getServiceType()
   {
      return mServiceType;
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
    * Gets the location for this device's UPnP interface description.
    * 
    * @return the location for the device's UPnP interface description.
    */
   public String getLocation()
   {
      return mLocation;
   }
   
   /**
    * Gets the string representation for this device.
    * 
    * @return the string representation for this device.
    */
   public String toString()
   {
      StringBuffer sb = new StringBuffer();
      
      sb.append("UPnPDevice[");
      
      sb.append("usn=" + getUsn());
      sb.append(",serviceType=" + getServiceType());
      sb.append(",server=" + getServer());
      sb.append(",location=" + getLocation());
      
      sb.append("]");
      
      return sb.toString();
   }
}
