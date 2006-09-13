/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.upnp;

import java.net.DatagramPacket;

import com.db.net.http.HttpWebResponseHeader;

/**
 * An SsdpDiscoverResponse is an HTTP based response to a SsdpDiscoverRequest
 * that uses the Simple Service Discovery Protocol (SSDP). It provides
 * information about a discovered device.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/draft_cai_ssdp_v1_03.txt
 * 
 * -----------------------------------------------------------------------
 * 4.2.1.1.  Example 
 * 
 * HTTP/1.1 200 OK
 * S: uuid:ijklmnop-7dec-11d0-a765-00a0c91e6bf6
 * Ext: 
 * Cache-Control: no-cache="Ext", max-age = 5000
 * ST: ge:fridge
 * USN: uuid:abcdefgh-7dec-11d0-a765-00a0c91e6bf6
 * AL: <blender:ixl><http://foo/bar>  *
 * -----------------------------------------------------------------------
 * 
 * @author Dave Longley
 */
public class SsdpDiscoverResponse
{
   /**
    * The HTTP web response header for this response.
    */
   protected HttpWebResponseHeader mHeader;
   
   /**
    * Creates a new SsdpDiscoverResponse from a datagram.
    * 
    * @param datagram the datagram to construct the response from.
    */
   public SsdpDiscoverResponse(DatagramPacket datagram)
   {
      // create the response header
      mHeader = new HttpWebResponseHeader();
      
      // get the header string
      String str = new String(
         datagram.getData(), datagram.getOffset(), datagram.getLength());
      
      // parse the header string
      mHeader.parse(str);
   }
   
   /**
    * Gets the HttpWebResponseHeader for this response.
    *
    * @return the HttpWebResponseHeader for this response.
    */
   public HttpWebResponseHeader getHeader()
   {
      return mHeader;
   }
   
   /**
    * Gets the usn for the device (header "usn").
    * 
    * @return the usn for the device.
    */
   public String getUsn()
   {
      String rval = "";
      
      String server = getHeader().getHeaderValue("usn");
      if(server != null)
      {
         rval = server.trim();
      }
      
      return rval;
   }   
   
   /**
    * Gets the service type (header "st").
    * 
    * @return the service type (a URI).
    */
   public String getServiceType()
   {
      String rval = "";
      
      String server = getHeader().getHeaderValue("st");
      if(server != null)
      {
         rval = server.trim();
      }
      
      return rval;
   }
   
   /**
    * Gets the server for the device (header "server").
    * 
    * @return the server for the device.
    */
   public String getServer()
   {
      String rval = "";
      
      String server = getHeader().getHeaderValue("server");
      if(server != null)
      {
         rval = server.trim();
      }
      
      return rval;
   }
   
   /**
    * Gets the location for the device's UPnP interface description
    * (header "location").
    * 
    * @return the location for the device.
    */
   public String getLocation()
   {
      String rval = "";
      
      String server = getHeader().getHeaderValue("location");
      if(server != null)
      {
         rval = server.trim();
      }
      
      return rval;
   }
   
   /**
    * Returns true if this response is valid (meaning it contains all of
    * the necessary headers), false if not.
    * 
    * @return true if this response is valid, false if not.
    */
   public boolean isValid()
   {
      boolean rval = false;

      if(getHeader().isValid())
      {
         // check header fields
         String usn = getUsn();
         if(usn != null && usn.trim().length() == 0)
         {
            String serviceType = getServiceType();
            if(serviceType != null && serviceType.trim().length() == 0)
            {
               String server = getServer();
               if(server != null && server.trim().length() == 0)
               {
                  String location = getLocation();
                  if(location != null && location.trim().length() == 0)
                  {
                     rval = true;
                  }
               }
            }
         }
         
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets this response in string format.
    * 
    * @return this response in string format.
    */
   public String toString()
   {
      return getHeader().toString();
   }
}
