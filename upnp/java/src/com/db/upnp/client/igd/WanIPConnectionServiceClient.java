/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client.igd;

import com.db.upnp.client.AbstractClientUPnPServiceImplementation;
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
extends AbstractClientUPnPServiceImplementation
{
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
      super(service);
   }
}
