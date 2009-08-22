/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client;

import com.db.upnp.service.UPnPServiceImplementation;

/**
 * This interface is for a class that actually provides the client-side
 * implementation for a UPnPService. 
 * 
 * @author Dave Longley
 */
public interface ClientUPnPServiceImplementation
extends UPnPServiceImplementation
{
   /**
    * Sets the UPnPServiceClient to use to communicate with the service.
    * 
    * @param client the UPnPServiceClient to use to communicate with the
    *               service.
    */
   public void setServiceClient(UPnPServiceClient client);
   
   /**
    * Gets the UPnPServiceClient to use to communicate with the service.
    * 
    * @return the UPnPServiceClient to use to communicate with the service.
    */
   public UPnPServiceClient getServiceClient();
}
