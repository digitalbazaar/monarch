/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.server.igd;

import com.db.upnp.server.ServerUPnPDeviceImplementation;

/**
 * A Internet Gateway Device. This is a device that provides a gateway
 * to the Internet. It has a service for Layer3Forwarding (Layer 3 is the
 * Network Layer in the OSI (Open Systems Interconnection) Model), that is,
 * it provides a service for forwarding particular ports to particular
 * network translated addresses.
 * 
 * @author Dave Longley
 */
public class InternetGatewayDevice implements ServerUPnPDeviceImplementation
{
   /**
    * Creates a new InternetGatewayDevice.
    */
   public InternetGatewayDevice()
   {
   }
}
