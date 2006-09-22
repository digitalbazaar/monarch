package com.db.upnp.client.igd;

import com.db.upnp.client.ClientUPnPDeviceImplementation;
import com.db.upnp.device.UPnPDevice;
import com.db.upnp.service.UPnPService;

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
    * The WanIPConnection service client for the device.
    */
   protected WanIPConnectionServiceClient mWanIPConnectionClient; 
   
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
      
      // get the WAN IP connection service client
      UPnPService service = device.getServiceList().getFirstService(
         WanIPConnectionServiceClient.WAN_IP_CONNECTION_SERVICE_TYPE);
      mWanIPConnectionClient =
         (WanIPConnectionServiceClient)service.getImplementation();
   }
   
   /**
    * Gets the WanIPConnectionServiceClient.
    * 
    * @return the WanIPConnectionServiceClient (can be null).
    */
   public WanIPConnectionServiceClient getWanIPConnectionServiceClient()
   {
      return mWanIPConnectionClient;
   }
}
