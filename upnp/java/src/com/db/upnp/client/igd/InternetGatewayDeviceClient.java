/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client.igd;

import com.db.upnp.client.ClientUPnPDeviceImplementation;
import com.db.upnp.device.UPnPDevice;

/**
 * A Internet Gateway Device Client. This is a client for an Internet Gateway
 * Device.
 * 
 * @author Dave Longley
 */
public class InternetGatewayDeviceClient
implements ClientUPnPDeviceImplementation
{
   /**
    * The UPnPDevice this implementation is for.
    */
   protected UPnPDevice mDevice;
   
   /**
    * The device type for an Internet Gateway Device.
    */
   public static final String IGD_DEVICE_TYPE =
      "urn:schemas-upnporg:device:InternetGatewayDevice:1";
   
   /**
    * Creates a new InternetGatewayDeviceClient for the specified UPnPDevice.
    * 
    * @param device the UPnPDevice to create this implementation for.
    */
   public InternetGatewayDeviceClient(UPnPDevice device)
   {
      // store device
      mDevice = device;
   }
}
