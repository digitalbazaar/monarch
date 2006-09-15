/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp;

/**
 * A UPnP Device. This object represents a device that supports the
 * UPnP protocol. It can be controlled via a UPnPControlPoint.
 * 
 * @author Dave Longley
 */
public class UPnPDevice
{
   /**
    * The server for this device.
    */
   protected String mServer;
   
   /**
    * The search target for this device.
    */
   protected String mSearchTarget;

   /**
    * The location (a URL) for this device's description.
    */
   protected String mLocation;
   
   /**
    * The Unique Service Name for the device.
    */
   protected String mUsn;
   
   /**
    * Creates a new UPnPDevice from a SsdpDiscoverResponse.
    * 
    * @param response the SsdpDiscoverResponse to create this device from.
    */
   public UPnPDevice(UPnPDiscoverResponse response)
   {
      // get device information
      mServer = response.getServer();
      mSearchTarget = response.getSearchTarget();
      mLocation = response.getLocation();
      mUsn = response.getUsn();
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
    * Gets the search target for this device.
    * 
    * @return the search target (a URI) for this device.
    */
   public String getSearchTarget()
   {
      return mSearchTarget;
   }
   
   /**
    * Gets the location (a URL) for this UPnP device's description.
    * 
    * @return the location for this device's description.
    */
   public String getLocation()
   {
      return mLocation;
   }
   
   /**
    * Gets the Unique Service Name for the device.
    * 
    * @return the Unique Service Name for the device.
    */
   public String getUsn()
   {
      return mUsn;
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
      
      sb.append("server=" + getServer());
      sb.append(",searchTarget=" + getSearchTarget());
      sb.append(",location=" + getLocation());
      sb.append(",usn=" + getUsn());
      
      sb.append("]");
      
      return sb.toString();
   }
}
