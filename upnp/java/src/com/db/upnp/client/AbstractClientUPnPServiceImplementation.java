/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client;

import com.db.upnp.service.UPnPService;

/**
 * An AbstractClientUPnPServiceImplementation provides the basic implementation
 * for a ClientUPnPServiceImplementation.
 * 
 * @author Dave Longley
 */
public class AbstractClientUPnPServiceImplementation
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
    * Creates a new AbstractClientUPnPServiceImplementation for the specified
    * UPnPService.
    * 
    * @param service the UPnPService this client implementation is for.
    */
   public AbstractClientUPnPServiceImplementation(UPnPService service)
   {
      // store service
      mService = service;
      
      // no service client set yet
      setServiceClient(null);
   }
   
   /**
    * Gets the UPnPService this client implementation is for.
    * 
    * @return the UPnPService this client implementation is for.
    */
   public UPnPService getService()
   {
      return mService;
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
