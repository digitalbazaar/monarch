/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client;

import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

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
   protected List<UPnPDevice> mCachedDevices;
   
   /**
    * Creates a new UPnPControlPoint.
    */
   public UPnPControlPoint()
   {
      // create the datagram server
      mServer = new DatagramServer();
      
      // create devices cache
      mCachedDevices = new LinkedList<UPnPDevice>();
   }
   
   /**
    * Causes this control point to start listening for UPnP devices that are
    * joining or leaving. 
    */
   public void startListeningForDevices()
   {
      // FIXME: implement me, use the datagram server
   }
   
   /**
    * Causes this control point to stop listening for UPnP devices that are
    * joining or leaving.  
    */
   public void stopListeningForDevices()
   {
      // FIXME: implement me, use the datagram server
   }
   
   /**
    * Discovers all root UPnP devices and caches them for use. This method may
    * take some time to execute.
    */
   public void discoverDevices()   
   {
      discoverDevices("upnp:rootdevice");
   }
   
   /**
    * Discovers a specific kind of device (i.e. of a specific model like a
    * dlink or linksys router) with the specified UDN and caches all found
    * devices of that kind for use. This method may take some time to execute.
    * 
    * @param udn the Unique Device Name for the specific device to discover.
    */
   public void discoverDevice(String udn)
   {
      discoverDevices(udn);
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
         // discover devices of the appropriate type
         UPnPDeviceDiscoverer discoverer = new UPnPDeviceDiscoverer();
         UPnPRootDevice[] devices = discoverer.discover(deviceType);
         
         for(int i = 0; i < devices.length; i++)
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
      catch(IOException e)
      {
         getLogger().error(getClass(),
            "Exception thrown while discovering devices!,exception=" + e);
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }
   }
   
   /**
    * Gets all of the devices discovered by this control point.
    * 
    * @return the UPnPDevices that have been discovered by this control point.
    */
   public List<UPnPDevice> getDiscoveredDevices()
   {
      return mCachedDevices;
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
   public List<UPnPDevice> getDiscoveredDevices(String deviceType)
   {
      List<UPnPDevice> devices = new LinkedList<UPnPDevice>();
      
      // go through the cached devices and add all devices with the given type
      for(UPnPDevice device: mCachedDevices)
      {
         // check the device type
         if(device.getDeviceType().equals(deviceType))
         {
            devices.add(device);
         }
         
         // go through embedded devices
         for(UPnPDevice embeddedDevice: device.getDeviceList())
         {
            if(embeddedDevice.getDeviceType().equals(deviceType))
            {
               devices.add(embeddedDevice);
            }
         }
      }
      
      return devices;
   }
   
   /**
    * Clears the cached devices from this control point.
    */
   public void clearCachedDevices()
   {
      mCachedDevices.clear();
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
