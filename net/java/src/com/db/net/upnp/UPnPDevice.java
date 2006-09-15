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
    * The search target for this device.
    */
   protected String mSearchTarget;
   
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
   public UPnPDevice(UPnPDiscoverResponse response)
   {
      // get device information
      mUsn = response.getUsn();
      mSearchTarget = response.getSearchTarget();
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
    * Gets the search target for this device.
    * 
    * @return the search target (a URI) for this device.
    */
   public String getSearchTarget()
   {
      return mSearchTarget;
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
      sb.append(",searchTarget=" + getSearchTarget());
      sb.append(",server=" + getServer());
      sb.append(",location=" + getLocation());
      
      sb.append("]");
      
      return sb.toString();
   }
}
