/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client;

import java.util.Iterator;

import com.db.upnp.device.UPnPDevice;
import com.db.upnp.device.UPnPRootDevice;
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
    * root device and sets it to the device, if the device does not already
    * have an implementation.
    * 
    * @param rootDevice the root device to create the implementations for.
    * 
    * @return true if the client implementation for the root device was
    *         created or false if not (i.e. no implementation exists for the
    *         device). 
    */
   public boolean createImplementation(UPnPRootDevice rootDevice)   
   {
      boolean rval = false;
      
      UPnPDevice device = rootDevice.getDescription().getDevice();
      if(device.getImplementation() == null)
      {
         // create a service client
         UPnPServiceClient serviceClient = new UPnPServiceClient(rootDevice);
         
         // create the device implementation
         rval = createImplementation(device, serviceClient);
      }
      
      return rval;
   }
   
   /**
    * Creates the appropriate ClientUPnPDeviceImplementation for the given
    * device and sets it to the device, if the device does not already
    * have an implementation.
    * 
    * @param device the device to create the implementation for.
    * @param serviceClient the service client for the device implementation.
    * 
    * @return true if the client implementation for the device was created
    *         or false if not (i.e. no implementation exists for the given
    *         device). 
    */
   public boolean createImplementation(
      UPnPDevice device, UPnPServiceClient serviceClient)
   {
      boolean rval = false;
      
      if(device.getImplementation() == null)
      {
         ClientUPnPDeviceImplementation implementation = null;
         
         // build the device implementation from the bottom up -- starting
         // with the device's embedded devices
         
         // create implementations for embedded devices
         for(Iterator i = device.getDeviceList().iterator(); i.hasNext();)
         {
            UPnPDevice embeddedDevice = (UPnPDevice)i.next();
            createImplementation(embeddedDevice, serviceClient);
         }

         // create a client service implementation factory
         ClientUPnPServiceImplementationFactory factory =
            new ClientUPnPServiceImplementationFactory();
         
         // create the service implementations
         factory.createImplementations(device, serviceClient);
         
         if(device.getDeviceType().equals(
            InternetGatewayDeviceClient.IGD_DEVICE_TYPE))
         {
            // create a internet gateway device client implementation
            implementation = new InternetGatewayDeviceClient(device);
         }
         else if(device.getDeviceType().equals(
                 WanConnectionDeviceClient.WAN_CONNECTION_DEVICE_TYPE))
         {
            // create a WAN connection client implementation
            implementation = new WanConnectionDeviceClient(device);
         }
         
         if(implementation != null)
         {
            // implementation created
            rval = true;
            
            // set the implementation to the device
            device.setImplementation(implementation);
         }
      }
      
      return rval;
   }
}
