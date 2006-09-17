/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client.igd;

import com.db.upnp.client.ClientUPnPServiceImplementation;
import com.db.upnp.client.UPnPServiceClient;
import com.db.upnp.service.UPnPService;

/**
 * A WanIPConnectionServiceClient is a client for a WanIPConnection Service.
 * 
 * FIXME: The implementation for this class is incomplete. Only port forwarding
 * functionality has been implemented.
 * 
 * @author Dave Longley
 */
public class WanIPConnectionServiceClient
implements ClientUPnPServiceImplementation
{
   /**
    * The UPnPService this client implementation is for.
    */
   protected UPnPService mService;

   /**
    * The UPnPServiceClient that is used to communicate with the UPnPService.
    */
   protected UPnPServiceClient mServiceClient;
   
   /**
    * The service type for a WAN IP Connection Service.
    */
   public static final String WAN_IP_CONNECTION_SERVICE_TYPE =
      "urn:schemas-upnp-org:service:WANIPConnection:1";
   
   /**
    * Creates a new WanIPConnectionServiceClient.
    * 
    * @param service the UPnPService this client implementation is for.
    */
   public WanIPConnectionServiceClient(UPnPService service)
   {
      // store service
      mService = service;
      
      // no service client set yet
      setServiceClient(null);
   }
   
   /**
    * Sets the UPnPServiceClient to use to communicate with the service.
    * 
    * @param client the UPnPServiceClient to use to communicate with the
    *               service.
    */
   public void setServiceClient(UPnPServiceClient client)
   {
      mServiceClient = client;
   }
   
   /**
    * Gets the UPnPServiceClient to use to communicate with the service.
    * 
    * @return the UPnPServiceClient to use to communicate with the service.
    */
   public UPnPServiceClient getServiceClient()
   {
      return mServiceClient;
   }
}
