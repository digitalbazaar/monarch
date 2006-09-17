/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client;

import java.util.Vector;

import com.db.net.datagram.DatagramServer;

/**
 * A UPnP Control Point. This is a single point that is capable of
 * controlling UPnPDevices via their UPnPServices.
 * 
 * @author Dave Longley
 */
public class UPnPControlPoint
{
   /**
    * A DatagramServer that is used to listen for joining/leaving UPnP devices
    * and events from UPnP devices.
    */
   protected DatagramServer mServer;
   
   /**
    * The UPnPDevices that this control point has cached for use.
    */
   protected Vector mCachedDevices;
   
   /**
    * Creates a new UPnPControlPoint.
    */
   public UPnPControlPoint()
   {
      // create the datagram server
      mServer = new DatagramServer();
      
      // create devices cache
      mCachedDevices = new Vector();
   }
   
   /**
    * Causes this control point to start listening for UPnP devices that are
    * joining or leaving. 
    */
   public void startListeningForDevices()
   {
      // FIXME: implement me
   }
   
   /**
    * Causes this control point to stop listening for UPnP devices that are
    * joining or leaving.  
    */
   public void stopListeningForDevices()
   {
      // FIXME: implement me
   }
   
   /**
    * Discovers devices with the specified type and caches them for use. This
    * method may take some time to execute.
    * 
    * @param deviceType the device type for the devices to discover.
    */
   public void discoverDevices(String deviceType)
   {
      // FIXME: implement me
   }
   
   /**
    * Gets the devices with the given device type that have been discovered by
    * this control point. 
    * 
    * @param deviceType the device type for the devices to return.
    * 
    * @return the UPnPDevices with the given device type that have been
    *         discovered by this control point.
    */
   public Vector getDiscoveredDevices(String deviceType)
   {
      Vector devices = new Vector();
      
      // FIXME: implement me
      
      return devices;
   }
}
