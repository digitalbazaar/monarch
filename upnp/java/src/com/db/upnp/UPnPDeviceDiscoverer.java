/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.SocketTimeoutException;
import java.util.Iterator;
import java.util.Vector;

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
    * @return a list of the discovered UPnPDevices as UPnPDiscoverResults.
    * 
    * @exception IOException thrown if an IO exception occurs.
    */
   public UPnPDiscoverResult[] discover() throws IOException
   {
      UPnPDiscoverResult[] results = null;
      
      // create a vector for storing discover results
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
      request.setSearchTarget("upnp:rootdevice");
      
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
               // create a UPnPDiscoverResult from the response
               UPnPDiscoverResult result = new UPnPDiscoverResult(response);
               discoveredDevices.add(result);
            }
         }
      }
      catch(SocketTimeoutException e)
      {
         // ignore timeout exception
      }

      // create results array
      results = new UPnPDiscoverResult[discoveredDevices.size()];
      int count = 0;
      for(Iterator i = discoveredDevices.iterator(); i.hasNext(); count++)
      {
         results[count] = (UPnPDiscoverResult)i.next();
      }
      
      // return results
      return results;
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
