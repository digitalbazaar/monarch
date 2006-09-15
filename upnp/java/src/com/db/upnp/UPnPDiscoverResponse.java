/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp;

import java.net.DatagramPacket;

import com.db.net.http.HttpWebResponseHeader;

/**
 * An UPnPDiscoverResponse is an HTTP based response to a UPnPDiscoverResponse
 * that uses the Simple Service Discovery Protocol (SSDP). It provides
 * information about a discovered UPnP device.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
 * -----------------------------------------------------------------------
 * HTTP/1.1 200 OK
 * CACHE-CONTROL: max-age = seconds until advertisement expires
 * DATE: when response was generated
 * EXT:
 * LOCATION: URL for UPnP description for root device
 * SERVER: OS/version UPnP/1.0 product/version
 * ST: search target
 * USN: advertisement UUID
 * -----------------------------------------------------------------------
 * Headers:
 * 
 * CACHE-CONTROL
 * Required. Must have max-age directive that specifies number of seconds the
 * advertisement is valid. After this duration, control points should assume
 * the device (or service) is no longer available. Should be > 1800 seconds
 * (30 minutes). Specified by UPnP vendor. Integer.
 * 
 * DATE
 * Recommended. When response was generated. RFC 1123 date.
 * 
 * EXT
 * Required. Confirms that the MAN header was understood.
 * (Header only; no value.)
 * 
 * LOCATION
 * Required. Contains a URL to the UPnP description of the root device. In
 * some unmanaged networks, host of this URL may contain an IP address
 * (versus a domain name). Specified by UPnP vendor. Single URL.
 * 
 * SERVER
 * Required. Concatenation of OS name, OS version, UPnP/1.0, product name,
 * and product version. Specified by UPnP vendor. String.
 * 
 * ST
 * Required header defined by SSDP. Search Target. Single URI. If ST header
 * in request was,
 * 
 * ssdp:all
 * Respond 3+2d+k times for a root device with d embedded devices and s
 * embedded services but only k distinct service types. Value for ST header
 * must be the same as for the NT header in NOTIFY messages with ssdp:alive.
 * (See above.) Single URI.
 * 
 * upnp:rootdevice
 * Respond once for root device. Must be upnp:rootdevice. Single URI.
 * 
 * uuid:device-UUID
 * Respond once for each device, root or embedded. Must be uuid:device-UUID.
 * Device UUID specified by UPnP vendor. Single URI.
 * 
 * urn:schemas-upnp-org:device:deviceType:v
 * Respond once for each device, root or embedded. Must be
 * urn:schemas-upnp-org:device:deviceType:v. Device type and version defined
 * by UPnP Forum working committee.
 * 
 * urn:schemas-upnp-org:service:serviceType:v
 * Respond once for each service. Must be
 * urn:schemas-upnp-org:service:serviceType:v. Service type and version defined
 * by UPnP Forum working committee.
 * 
 * USN
 * Required header defined by SSDP. Unique Service Name. (See list of required
 * values for USN header in NOTIFY with ssdp:alive above.) Single URI. 
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
    * Gets the number of seconds the advertisement is available.
    * 
    * @return the number of seconds the advertisement is available.
    */
   public int getCacheControl()
   {
      int rval = 0;
      
      if(getHeader().hasHeader("cache-control"))
      {
         try
         {
            rval = Integer.parseInt(
               getHeader().getHeaderValue("cache-control"));
         }
         catch(Throwable ignore)
         {
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the date when the response was generated.
    * 
    * @return the date when the response was generated.
    */
   public String getDate()
   {
      String rval = "";
      
      if(getHeader().hasHeader("date"))
      {
         rval = getHeader().getHeaderValue("date");
      }
      
      return rval;
   }
   
   /**
    * Gets the location (a URL) for the UPnP device description.
    * 
    * @return the location for the device description.
    */
   public String getLocation()
   {
      String rval = "";
      
      if(getHeader().hasHeader("location"))
      {
         rval = getHeader().getHeaderValue("location").trim();
      }
      
      return rval;
   }   
   
   /**
    * Gets the server for the device.
    * 
    * @return the server for the device.
    */
   public String getServer()
   {
      String rval = "";
      
      if(getHeader().hasHeader("server"))
      {
         rval = getHeader().getHeaderValue("server").trim();
      }
      
      return rval;
   }   
   
   /**
    * Gets the search target.
    * 
    * @return the search target (a URI).
    */
   public String getSearchTarget()
   {
      String rval = "";
      
      if(getHeader().hasHeader("st"))
      {
         rval = getHeader().getHeaderValue("st").trim();
      }
      
      return rval;
   }
   
   /**
    * Gets the Unique Service Name for the device.
    * 
    * @return the Unique Service Name for the device.
    */
   public String getUsn()
   {
      String rval = "";
      
      if(getHeader().hasHeader("usn"))
      {
         rval = getHeader().getHeaderValue("usn").trim();
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
         // these headers must be present
         if(getHeader().hasHeader("cache-control") &&
            getHeader().hasHeader("ext") &&
            getHeader().hasHeader("location") &&
            getHeader().hasHeader("server") &&
            getHeader().hasHeader("st") &&
            getHeader().hasHeader("usn"))
         {
            rval = true;
         }
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
