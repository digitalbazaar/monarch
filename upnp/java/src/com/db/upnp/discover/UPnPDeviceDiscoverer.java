/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.discover;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.SocketTimeoutException;
import java.util.Iterator;
import java.util.Vector;

import com.db.net.datagram.DatagramClient;
import com.db.net.datagram.DatagramStream;
import com.db.upnp.device.UPnPRootDevice;

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
    * Sends an SSDP message to discover all UPnPRootDevices.
    * 
    * @return a list of the discovered UPnPRootDevices.
    * 
    * @exception IOException thrown if an IO exception occurs.
    */
   public UPnPRootDevice[] discover() throws IOException   
   {
      return discover("upnp:rootdevice");
   }
   
   /**
    * Sends an SSDP message to discover UPnP devices of a specific type. The
    * passed search target URI can be "ssdb:all", "upnp:rootdevice", or a
    * device type or UDN.
    * 
    * @param searchTarget the search target URI that indicates which devices
    *                     to discover.
    * 
    * @return a list of the discovered UPnPRootDevices that have devices that
    *         match the given search target URI.
    * 
    * @exception IOException thrown if an IO exception occurs.
    */
   public UPnPRootDevice[] discover(String searchTarget) throws IOException
   {
      UPnPRootDevice[] devices = null;
      
      // create a vector for storing discovered devices
      Vector discoveredDevices = new Vector(); 
      
      // get a multicast stream
      DatagramClient client = new DatagramClient();
      DatagramStream stream = client.getMulticastStream(
         UPnPDiscoverRequest.SSDP_MULTICAST_ADDRESS,
         UPnPDiscoverRequest.UPnP_SSDP_PORT, false);
      
      // set the timeout to the discovery timeout
      stream.setReadTimeout(getTimeout());
      
      // create a discover request
      UPnPDiscoverRequest request = new UPnPDiscoverRequest();
      
      // set the search target
      request.setSearchTarget(searchTarget);
      
      // create a datagram
      byte[] bytes = request.getBytes();
      DatagramPacket datagram = new DatagramPacket(bytes, bytes.length);
      
      // send the datagram
      stream.sendDatagram(datagram);
      
      // FIXME: we need to set up another stream to listen for the results
      // because they may come back before we can start receiving datagrams
      try
      {
         // receive packets until timeout is reached
         while(true)
         {
            byte[] buffer = new byte[2048];
            datagram = new DatagramPacket(buffer, buffer.length);
            stream.receiveDatagram(datagram);
            
            // parse a discover response from the datagram
            UPnPDiscoverResponse response = new UPnPDiscoverResponse(datagram);
            if(response.isValid())
            {
               // create a UPnPRootDevice from the response
               UPnPRootDevice device = new UPnPRootDevice();
               
               // get device information
               device.setServer(response.getServer());
               device.setLocation(response.getLocation());
               device.setUsn(response.getUsn());
               
               // add device to list of discovered devices
               discoveredDevices.add(device);
            }
         }
      }
      catch(SocketTimeoutException e)
      {
         // ignore timeout exception
      }

      // create devices array
      devices = new UPnPRootDevice[discoveredDevices.size()];
      int count = 0;
      for(Iterator i = discoveredDevices.iterator(); i.hasNext(); count++)
      {
         devices[count] = (UPnPRootDevice)i.next();
      }
      
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
