/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.upnp;

/**
 * A UPnP Device Discoverer. This is an object that is capable of
 * discovering UPnP devices using SSDP (Simple Service Discovery Protocol).
 * 
 * @author Dave Longley
 */
public class UPnPDeviceDiscoverer
{
   /**
    * The multicast address for SSDP.
    */
   protected static final String SSDP_MULTICAST_ADDRESS = "239.255.255.250";
   
   /**
    * The UPnP UDP port.
    */
   protected static final int UPnP_UDP_PORT = 1900;
   
   /**
    * The UPnP TCP port.
    */
   protected static final int UPnP_TCP_PORT = 5000;
   
   /**
    * Creates a new UPnPDeviceDiscoverer.
    */
   public UPnPDeviceDiscoverer()
   {
   }
}
