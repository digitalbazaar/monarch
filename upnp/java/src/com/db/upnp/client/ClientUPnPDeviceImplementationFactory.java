/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client;

import com.db.upnp.device.UPnPDevice;
import com.db.upnp.client.igd.InternetGatewayDeviceClient;
import com.db.upnp.client.igd.WanConnectionDeviceClient;

/**
 * A ClientUPnPDeviceImplementationFactory is a factory used to produce
 * ClientUPnPDeviceImplementations for UPnPDevices. 
 * 
 * @author Dave Longley
 */
public class ClientUPnPDeviceImplementationFactory
{
   /**
    * Creates a new ClientUPnPDeviceImplementationFactory.
    */
   public ClientUPnPDeviceImplementationFactory()
   {
   }
   
   /**
    * Creates the appropriate ClientUPnPDeviceImplementation for the given
    * device and sets it to the device.
    * 
    * @param device the device to create the implementation for.
    * 
    * @return true if the client implementation for the device was created
    *         or false if no implementation exists for the given device. 
    */
   public boolean createImplementation(UPnPDevice device)
   {
      boolean rval = false;
      
      if(device.getDeviceType().equals(
         InternetGatewayDeviceClient.IGD_DEVICE_TYPE))
      {
         // create a internet gateway device client implementation
         InternetGatewayDeviceClient implementation =
            new InternetGatewayDeviceClient(device);
         
         // set the implementation to the device
         device.setImplementation(implementation);
      }
      else if(device.getDeviceType().equals(
               WanConnectionDeviceClient.WAN_CONNECTION_DEVICE_TYPE))
      {
         // create a WAN connection client implementation
         WanConnectionDeviceClient implementation =
            new WanConnectionDeviceClient(device);
         
         // set the implementation to the device
         device.setImplementation(implementation);
      }
      
      return rval;
   }
}
