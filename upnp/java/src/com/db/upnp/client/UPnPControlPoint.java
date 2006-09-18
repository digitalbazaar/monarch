/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client;

import java.io.IOException;
import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.datagram.DatagramServer;
import com.db.upnp.device.UPnPDevice;
import com.db.upnp.device.UPnPRootDevice;
import com.db.upnp.discover.UPnPDeviceDiscoverer;

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
      try
      {
         // FIXME: change this up -- we don't want to always retrieve
         // descriptions and create implementations, we only want to do so
         // if we're interested
         UPnPDeviceDiscoverer discoverer = new UPnPDeviceDiscoverer();
         UPnPRootDevice[] devices = discoverer.discover();
         
         for(int i = 0; i < devices.length; i++)
         {
            if(devices[i].getLocation().startsWith("http://5"))
            {
            
            // retrieve the description for the root device
            if(devices[i].retrieveDeviceDescription())
            {
               // retrieve all descriptions for the device
               devices[i].retrieveAllDescriptions();
               
               // create implementation for the device
               ClientUPnPDeviceImplementationFactory factory =
                  new ClientUPnPDeviceImplementationFactory();
               factory.createImplementation(devices[i]);
               
               // add the device
               mCachedDevices.add(devices[i].getDescription().getDevice());
            }
            }
         }
      }
      catch(IOException e)
      {
         getLogger().error(getClass(),
            "Exception thrown while discovering devices!,exception=" + e);
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }
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
      
      // go through the cached devices and add all devices with the given type
      for(Iterator i = mCachedDevices.iterator(); i.hasNext();)
      {
         UPnPDevice device = (UPnPDevice)i.next();
         
         // check the device type
         if(device.getDeviceType().equals(deviceType))
         {
            devices.add(device);
         }
         
         // go through embedded devices
         for(Iterator id = device.getDeviceList().iterator(); id.hasNext();)
         {
            UPnPDevice embeddedDevice = (UPnPDevice)id.next();
            if(embeddedDevice.getDeviceType().equals(deviceType))
            {
               devices.add(embeddedDevice);
            }
         }
      }
      
      return devices;
   }
   
   /**
    * Gets the logger for this UPnPControlPoint.
    * 
    * @return the logger for this UPnPControlPoint.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
