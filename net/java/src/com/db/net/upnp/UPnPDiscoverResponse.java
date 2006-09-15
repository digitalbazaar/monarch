/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.upnp;

import java.net.DatagramPacket;

import com.db.net.http.HttpWebResponseHeader;

/**
 * An UPnPDiscoverResponse is an HTTP based response to a UPnPDiscoverResponse
 * that uses the Simple Service Discovery Protocol (SSDP). It provides
 * information about a discovered UPnP device.
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
public class UPnPDiscoverResponse
{
   /**
    * The HTTP web response header for this response.
    */
   protected HttpWebResponseHeader mHeader;
   
   /**
    * Creates a new UPnPDiscoverResponse from a datagram.
    * 
    * @param datagram the datagram to construct the response from.
    */
   public UPnPDiscoverResponse(DatagramPacket datagram)
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
      
      String usn = getHeader().getHeaderValue("usn");
      if(usn != null)
      {
         rval = usn.trim();
      }
      
      return rval;
   }   
   
   /**
    * Gets the search target (header "st").
    * 
    * @return the search target (a URI).
    */
   public String getSearchTarget()
   {
      String rval = "";
      
      String searchTarget = getHeader().getHeaderValue("st");
      if(searchTarget != null)
      {
         rval = searchTarget.trim();
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
      
      String location = getHeader().getHeaderValue("location");
      if(location != null)
      {
         rval = location.trim();
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
            String serviceType = getSearchTarget();
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
