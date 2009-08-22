/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client.igd;

import com.db.upnp.client.ClientUPnPDeviceImplementation;
import com.db.upnp.device.UPnPDevice;

/**
 * A WAN Connection Device Client. This is a client for a WAN Connection Device.
 * 
 * @author Dave Longley
 */
public class WanDeviceClient
implements ClientUPnPDeviceImplementation
{
   /**
    * The UPnPDevice this implementation is for.
    */
   protected UPnPDevice mDevice;
   
   /**
    * The embedded WanConnectionDevice client.
    */
   protected WanConnectionDeviceClient mWanConnectionDeviceClient; 
   
   /**
    * The device type for a WAN Device.
    */
   public static final String WAN_DEVICE_TYPE =
      "urn:schemas-upnp-org:device:WANDevice:1";
   
   /**
    * Creates a new WanDeviceClient for the specified UPnPDevice.
    * 
    * @param device the UPnPDevice to create this implementation for.
    */
   public WanDeviceClient(UPnPDevice device)
   {
      // store device
      mDevice = device;
      
      // get the embedded WAN connection device client
      UPnPDevice embedded = device.getDeviceList().getFirstDevice(
         WanConnectionDeviceClient.WAN_CONNECTION_DEVICE_TYPE);
      mWanConnectionDeviceClient =
         (WanConnectionDeviceClient)embedded.getImplementation();      
   }
   
   /**
    * Gets the WanConnectionDeviceClient.
    * 
    * @return the WanConnectionDeviceClient (can be null).
    */
   public WanConnectionDeviceClient getWanConnectionDeviceClient()
   {
      return mWanConnectionDeviceClient;
   }   
   
   /**
    * Gets the WanIPConnectionServiceClient.
    * 
    * @return the WanIPConnectionServiceClient (can be null).
    */
   public WanIPConnectionServiceClient getWanIPConnectionServiceClient()
   {
      WanIPConnectionServiceClient rval = null;
      
      if(getWanConnectionDeviceClient() != null)
      {
         rval = getWanConnectionDeviceClient().
            getWanIPConnectionServiceClient();
      }
      
      return rval;      
   }
}
