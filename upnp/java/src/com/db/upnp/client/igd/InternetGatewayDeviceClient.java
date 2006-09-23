/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client.igd;

import java.net.ConnectException;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.upnp.client.ClientUPnPDeviceImplementation;
import com.db.upnp.device.UPnPDevice;
import com.db.upnp.service.UPnPErrorException;
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
    * Adds a port mapping to the Internet Gateway Device.
    * 
    * @param portMapping the port mapping to add.
    * 
    * @exception UPnPErrorException thrown if a UPnPError occurs.
    */
   public void addPortMapping(PortMapping portMapping)
   throws UPnPErrorException
   {
      try
      {
         getWanIPConnectionServiceClient().addPortMapping(portMapping);
      }
      catch(NullPointerException e)
      {
         // log exception
         getLogger().error(getClass(),
            "Could not add port mapping, " +
            "no port mapping service implementation found!");
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }      
      catch(ConnectException e)
      {
         // log exception
         getLogger().error(getClass(),
            "Could not add port mapping, could not connect to service!");
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }
   }
   
   /**
    * Removes a port mapping from the Internet Gateway Device.
    * 
    * @param portMapping the port mapping to remove.
    * 
    * @exception UPnPErrorException thrown if a UPnPError occurs.
    */
   public void removePortMapping(PortMapping portMapping)
   throws UPnPErrorException   
   {
      removePortMapping(
         portMapping.getRemoteHost(),
         portMapping.getExternalPort(),
         portMapping.getProtocol());
   }
   
   /**
    * Removes a port mapping from the Internet Gateway Device.
    * 
    * @param remoteHost the remote host (an IP address "x.x.x.x" as a string).
    * @param externalPort the external port.
    * @param protocol the protocol ("TCP" or "UDP").
    * 
    * @exception UPnPErrorException thrown if a UPnPError occurs.
    */
   public void removePortMapping(
      String remoteHost, int externalPort, String protocol)
   throws UPnPErrorException
   {
      try
      {
         getWanIPConnectionServiceClient().deletePortMapping(
            remoteHost, externalPort, protocol);
      }
      catch(NullPointerException e)
      {
         // log exception
         getLogger().error(getClass(),
            "Could not remove port mapping, " +
            "no port mapping service implementation found!");
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }      
      catch(ConnectException e)
      {
         // log exception
         getLogger().error(getClass(),
            "Could not remove port mapping, could not connect to service!");
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }
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
   
   /**
    * Gets the logger for this InternetGatewayDeviceClient.
    * 
    * @return the logger for this InternetGatewayDeviceClient.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }   
}
