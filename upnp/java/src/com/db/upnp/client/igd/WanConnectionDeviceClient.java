package com.db.upnp.client.igd;

import com.db.upnp.client.ClientUPnPDeviceImplementation;
import com.db.upnp.device.UPnPDevice;

/**
 * A WAN Connection Device Client. This is a client for a WAN Connection Device.
 * 
 * @author Dave Longley
 */
public class WanConnectionDeviceClient
implements ClientUPnPDeviceImplementation
{
   /**
    * The UPnPDevice this implementation is for.
    */
   protected UPnPDevice mDevice;
   
   /**
    * The device type for a WAN Connection Device.
    */
   public static final String WAN_CONNECTION_DEVICE_TYPE =
      "urn:schemas-upnp-org:device:WANConnectionDevice:1";
   
   /**
    * Creates a new WanConnectionDeviceClient for the specified UPnPDevice.
    * 
    * @param device the UPnPDevice to create this implementation for.
    */
   public WanConnectionDeviceClient(UPnPDevice device)
   {
      // store device
      mDevice = device;
   }
}
