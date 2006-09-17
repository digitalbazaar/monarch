/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.server;

import com.db.upnp.service.UPnPService;

/**
 * A ServerUPnPServiceImplementationFactory is a factory used to produce
 * ServerUPnPServiceImplementations for UPnPServices. 
 * 
 * @author Dave Longley
 */
public class ServerUPnPServiceImplementationFactory
{
   /**
    * Creates a new ServerUPnPServiceImplementationFactory.
    */
   public ServerUPnPServiceImplementationFactory()
   {
   }
   
   /**
    * Creates the appropriate ServerUPnPServiceImplementation for the given
    * service and sets it to the service.
    * 
    * @param service the service to create the implementation for.
    * 
    * @return true if the server implementation for the service was created
    *         or false if no implementation exists for the given service. 
    */
   public boolean createImplementation(UPnPService service)
   {
      boolean rval = false;
      
      // FIXME: implement me
      
      return rval;
   }
}
