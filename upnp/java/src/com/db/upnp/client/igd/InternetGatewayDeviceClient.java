/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client.igd;

import com.db.upnp.client.ClientUPnPDeviceImplementation;
import com.db.upnp.device.UPnPDevice;
import com.db.upnp.service.UPnPService;

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
    * The Layer3Forwarding service client for the device.
    */
   protected Layer3ForwardingServiceClient mLayer3ForwardingClient;
   
   /**
    * The embedded WanDevice client.
    */
   protected WanDeviceClient mWanDeviceClient; 
   
   /**
    * The device type for an Internet Gateway Device.
    */
   public static final String IGD_DEVICE_TYPE =
      "urn:schemas-upnp-org:device:InternetGatewayDevice:1";
   
   /**
    * Creates a new InternetGatewayDeviceClient for the specified UPnPDevice.
    * 
    * @param device the UPnPDevice to create this implementation for.
    */
   public InternetGatewayDeviceClient(UPnPDevice device)
   {
      // store device
      mDevice = device;
      
      // get the layer 3 forwarding service client
      UPnPService service = device.getServiceList().getFirstService(
         Layer3ForwardingServiceClient.LAYER3_FORWARDING_SERVICE_TYPE);
      mLayer3ForwardingClient =
         (Layer3ForwardingServiceClient)service.getImplementation();
      
      // get the embedded WAN device client
      UPnPDevice embedded = device.getDeviceList().getFirstDevice(
         WanDeviceClient.WAN_DEVICE_TYPE);
      mWanDeviceClient = (WanDeviceClient)embedded.getImplementation();
   }
   
   /**
    * Gets the Layer3ForwardingServiceClient.
    * 
    * @return the Layer3ForwardingServiceClient (can be null).
    */
   public Layer3ForwardingServiceClient getLayer3ForwardingServiceClient()
   {
      return mLayer3ForwardingClient;
   }
   
   /**
    * Gets the WanDeviceClient.
    * 
    * @return the WanDeviceClient (can be null).
    */
   public WanDeviceClient getWanDeviceClient()
   {
      return mWanDeviceClient;
   }
   
   /**
    * Gets the WanIPConnectionServiceClient.
    * 
    * @return the WanIPConnectionServiceClient (can be null).
    */
   public WanIPConnectionServiceClient getWanIPConnectionServiceClient()
   {
      WanIPConnectionServiceClient rval = null;
      
      if(getWanDeviceClient() != null)
      {
         rval = getWanDeviceClient().getWanIPConnectionServiceClient();
      }
      
      return rval;
   }   
}
