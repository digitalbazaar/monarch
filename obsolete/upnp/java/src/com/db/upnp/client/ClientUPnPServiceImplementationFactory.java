/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client;

import java.util.Iterator;

import com.db.upnp.client.igd.Layer3ForwardingServiceClient;
import com.db.upnp.client.igd.WanIPConnectionServiceClient;
import com.db.upnp.device.UPnPDevice;
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
    * Creates an appropriate ClientUPnPServiceImplementation for all of the
    * services for the given UPnPDevice that do not yet have implementations.
    * Only some of the device's services may be implemented if implementations
    * do not exists for all of the services.
    * 
    * Implementations for services for embedded devices will not be created. 
    * 
    * @param device the UPnPDevice to create service implementations for.
    * @param serviceClient the UPnPServiceClient for the service implementation.
    * 
    * @return true if all of the device's services were implemented, false
    *         if not.
    */
   public boolean createImplementations(
      UPnPDevice device, UPnPServiceClient serviceClient)   
   {
      boolean rval = true;
      
      // create implementations for each service
      for(Iterator i = device.getServiceList().iterator(); i.hasNext();)
      {
         UPnPService service = (UPnPService)i.next();
         rval &= createImplementation(service, serviceClient);
      }
      
      return rval;
   }
   
   /**
    * Creates the appropriate ClientUPnPServiceImplementation for the given
    * service and sets it to the service, if the service does not already
    * have a set implementation.
    * 
    * @param service the service to create the implementation for.
    * @param serviceClient the UPnPServiceClient for the service implementation.
    * 
    * @return true if the client implementation for the service was created
    *         or false if not (i.e. no implementation exists for the given
    *         service). 
    */
   public boolean createImplementation(
      UPnPService service, UPnPServiceClient serviceClient)
   {
      boolean rval = false;
      
      if(service.getImplementation() == null)
      {
         ClientUPnPServiceImplementation implementation = null;
         
         if(service.getServiceType().equals(
            Layer3ForwardingServiceClient.LAYER3_FORWARDING_SERVICE_TYPE))
         {
            // create a Layer 3 forwarding service client implementation
            implementation = new Layer3ForwardingServiceClient(service);
         }
         else if(service.getServiceType().equals(
                 WanIPConnectionServiceClient.WAN_IP_CONNECTION_SERVICE_TYPE))
         {
            // create WAN IP Connection service client implementation
            implementation = new WanIPConnectionServiceClient(service);
         }
         
         if(implementation != null)
         {
            // set the service client for the implementation
            implementation.setServiceClient(serviceClient);
               
            // set the implementation to the service
            service.setImplementation(implementation);
         }
      }
      
      return rval;
   }
}
