/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.server;

import com.db.upnp.server.igd.InternetGatewayDevice;
import com.db.upnp.device.UPnPDevice;

/**
 * A ServerUPnPDeviceImplementationFactory is a factory used to produce
 * ServerUPnPDeviceImplementations for UPnPDevices. 
 * 
 * @author Dave Longley
 */
public class ServerUPnPDeviceImplementationFactory
{
   /**
    * Creates a new ServerUPnPDeviceImplementationFactory.
    */
   public ServerUPnPDeviceImplementationFactory()
   {
   }
   
   /**
    * Creates the appropriate ServerUPnPDeviceImplementation for the given
    * device and sets it to the device, if the device does not already
    * have an implementation.
    * 
    * @param device the device to create the implementation for.
    * 
    * @return true if the server implementation for the device was created
    *         or false if not (i.e. no implementation exists for the given
    *         device). 
    */
   public boolean createImplementation(UPnPDevice device)
   {
      boolean rval = false;
      
      if(device.getImplementation() == null)
      {
         ServerUPnPDeviceImplementation implementation = null;
         
         if(device.getDeviceType().equals(
            InternetGatewayDevice.IGD_DEVICE_TYPE))
         {
            // create a internet gateway device implementation
            implementation = new InternetGatewayDevice(device);
         }
         
         if(implementation != null)
         {
            // set the implementation to the device
            device.setImplementation(implementation);
         }
      }
      
      return rval;
   }   
}
