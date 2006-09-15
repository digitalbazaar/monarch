/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp;

/**
 * A UPnPDeviceResult represents a discovered UPnPDevice. It has basic
 * information about the device.
 * 
 * @author Dave Longley
 */
public class UPnPDiscoverResult
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
    * Creates a new UPnPDiscoverResult from a UPnPDiscoverResponse.
    * 
    * @param response the UPnPDiscoverResponse to create this result from.
    */
   public UPnPDiscoverResult(UPnPDiscoverResponse response)
   {
      // get device information
      mServer = response.getServer();
      mSearchTarget = response.getSearchTarget();
      mLocation = response.getLocation();
      mUsn = response.getUsn();
   }
   
   /**
    * Gets the server for this result.
    * 
    * @return the server for this result.
    */
   public String getServer()
   {
      return mServer;
   }   

   /**
    * Gets the search target for this result.
    * 
    * @return the search target (a URI) for this result.
    */
   public String getSearchTarget()
   {
      return mSearchTarget;
   }
   
   /**
    * Gets the location (a URL) for the UPnP device's description.
    * 
    * @return the location for the device's description.
    */
   public String getLocation()
   {
      return mLocation;
   }
   
   /**
    * Gets the Unique Service Name for the UPnP device.
    * 
    * @return the Unique Service Name for the UPnP device.
    */
   public String getUsn()
   {
      return mUsn;
   }   
   
   /**
    * Gets the string representation for this result.
    * 
    * @return the string representation for this result.
    */
   public String toString()
   {
      StringBuffer sb = new StringBuffer();
      
      sb.append("[UPnPDiscoverResult]");
      
      sb.append("\nserver=" + getServer());
      sb.append("\nsearchTarget=" + getSearchTarget());
      sb.append("\nlocation=" + getLocation());
      sb.append("\nusn=" + getUsn());
      
      sb.append("\n");
      
      return sb.toString();
   }
}
