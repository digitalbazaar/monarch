/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client;

import com.db.upnp.client.igd.Layer3ForwardingServiceClient;
import com.db.upnp.client.igd.WanIPConnectionServiceClient;
import com.db.upnp.service.UPnPService;

/**
 * A ClientUPnPServiceImplementationFactory is a factory used to produce
 * ClientUPnPServiceImplementations for UPnPServices. 
 * 
 * @author Dave Longley
 */
public class ClientUPnPServiceImplementationFactory
{
   /**
    * Creates a new ClientUPnPServiceImplementationFactory.
    */
   public ClientUPnPServiceImplementationFactory()
   {
   }
   
   /**
    * Creates the appropriate ClientUPnPServiceImplementation for the given
    * service and sets it to the service.
    * 
    * @param service the service to create the implementation for.
    * 
    * @return true if the client implementation for the service was created
    *         or false if no implementation exists for the given service. 
    */
   public boolean createImplementation(UPnPService service)
   {
      boolean rval = false;
      
      if(service.getServiceType().equals(
         Layer3ForwardingServiceClient.LAYER3_FORWARDING_SERVICE_TYPE))
      {
         // create a Layer 3 forwarding service client implementation
         Layer3ForwardingServiceClient implementation =
            new Layer3ForwardingServiceClient(service);
            
         // set the implementation to the service
         service.setImplementation(implementation);
      }
      else if(service.getServiceType().equals(
              WanIPConnectionServiceClient.WAN_IP_CONNECTION_SERVICE_TYPE))
      {
         // create WAN IP Connection service client implementation
         WanIPConnectionServiceClient implementation =
            new WanIPConnectionServiceClient(service);
         
         // set the implementation to the service
         service.setImplementation(implementation);
      }
      
      return rval;
   }
}
