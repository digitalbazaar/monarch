/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.upnp;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.SocketTimeoutException;

import com.db.net.datagram.DatagramClient;
import com.db.net.datagram.DatagramStream;

/**
 * A UPnP Device Discoverer. This is an object that is capable of
 * discovering UPnP devices using SSDP (Simple Service Discovery Protocol).
 * 
 * @author Dave Longley
 */
public class UPnPDeviceDiscoverer
{
   /**
    * The UPnP TCP port.
    */
   protected static final int UPnP_TCP_PORT = 5000;
   
   /**
    * The timeout (in milliseconds) for device discovery.
    */
   protected int mDiscoveryTimeout;
   
   /**
    * Creates a new UPnPDeviceDiscoverer.
    */
   public UPnPDeviceDiscoverer()
   {
      // set default discovery timeout (2 seconds)
      mDiscoveryTimeout = 2000;
   }
   
   /**
    * Sends an SSDP message to discover UPnP devices.
    * 
    * @return all of the discovered UPnPDevices.
    * 
    * @exception IOException thrown if an IO exception occurs.
    */
   public UPnPDevice[] discover() throws IOException
   {
      UPnPDevice[] devices = new UPnPDevice[0];
      
      // get a multicast stream
      DatagramClient client = new DatagramClient();
      DatagramStream stream = client.getMulticastStream(
         SsdpDiscoverRequest.SSDP_MULTICAST_ADDRESS,
         SsdpDiscoverRequest.UPnP_SSDP_PORT, false);
      
      // set the timeout to the discovery timeout
      stream.setReadTimeout(getTimeout());
      
      // create a SSDP request
      SsdpDiscoverRequest request = new SsdpDiscoverRequest();
      
      // set the service type
      request.setServiceType("upnp:rootdevice");
      
      // create a datagram
      byte[] bytes = request.getBytes();
      DatagramPacket datagram = new DatagramPacket(bytes, bytes.length);
      
      // send the datagram
      stream.sendDatagram(datagram);
      
      try
      {
         // receive packets until timeout is reached
         while(true)
         {
            byte[] buffer = new byte[2048];
            datagram = new DatagramPacket(buffer, buffer.length);
            stream.receiveDatagram(datagram);
            
            // parse a SsdpDiscoverResponse from the datagram
            SsdpDiscoverResponse response = new SsdpDiscoverResponse(datagram);
            System.out.println(response.toString());
         }
      }
      catch(SocketTimeoutException e)
      {
         // ignore timeout exception
      }

      
      // FIXME:
      
      // return devices
      return devices;
   }
   
   /**
    * Sets the discovery timeout. This must be set before calling discover()
    * to have any effect.
    * 
    * @param timeout the discovery timeout (in milliseconds) > 0.
    */
   public void setTimeout(int timeout)
   {
      mDiscoveryTimeout = Math.max(1, timeout);
   }
   
   /**
    * Gets the discovery timeout.
    * 
    * @return the discovery timeout (in milliseconds) > 0.
    */
   public int getTimeout()
   {
      return mDiscoveryTimeout;
   }
}
