/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.upnp;

import com.db.net.http.HttpWebRequestHeader;

/**
 * An UPnPDiscoverRequest is an HTTP based request that uses the
 * Simple Service Discovery Protocol (SSDP) to discover UPnP devices.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
 * -----------------------------------------------------------------------
 * M-SEARCH * HTTP/1.1
 * HOST: 239.255.255.250:1900
 * MAN: "ssdp:discover"
 * MX: seconds to delay response
 * ST: search target
 * -----------------------------------------------------------------------
 * Request line
 *  
 * M-SEARCH 
 * Method defined by SSDP for search requests. 
 * Request applies generally and not to a specific resource. Must be *.
 * 
 * HTTP/1.1 
 * HTTP version.
 *  
 * Headers:
 * 
 * HOST 
 * Required. Multicast channel and port reserved for SSDP by Internet Assigned
 * Numbers Authority (IANA). Must be 239.255.255.250:1900.
 *  
 * MAN 
 * Required. Unlike the NTS and ST headers, the value of the MAN header is
 * enclosed in double quotes. Must be "ssdp:discover". 
 * 
 * MX
 * Required. Maximum wait. Device responses should be delayed a random duration
 * between 0 and this many seconds to balance load for the control point when
 * it processes responses. This value should be increased if a large number of
 * devices are expected to respond or if network latencies are expected to be
 * significant. Specified by UPnP vendor. Integer.
 * 
 * ST
 * Required header defined by SSDP. Search Target. Must be one of the following.
 * (cf. NT header in NOTIFY with ssdp:alive above.) Single URI.
 * 
 * ssdp:all
 * Search for all devices and services.
 * 
 * upnp:rootdevice
 * Search for root devices only.
 * 
 * uuid:device-UUID 
 * Search for a particular device. Device UUID specified by UPnP vendor.
 * 
 * urn:schemas-upnp-org:device:deviceType:v
 * Search for any device of this type. Device type and version defined by UPnP
 * Forum working committee.
 * 
 * urn:schemas-upnp-org:service:serviceType:v
 * Search for any service of this type. Service type and version defined by
 * UPnP Forum working committee.
 * -----------------------------------------------------------------------
 * 
 * @author Dave Longley
 */
public class UPnPDiscoverRequest
{
   /**
    * The HTTP web request header for this request.
    */
   protected HttpWebRequestHeader mHeader;
   
   /**
    * The multicast address for SSDP.
    */
   public static final String SSDP_MULTICAST_ADDRESS = "239.255.255.250";
   
   /**
    * The UPnP SSDP port for device discovery.
    */
   public static final int UPnP_SSDP_PORT = 1900;
   
   /**
    * Creates a new UPnPDiscoverRequest.
    */
   public UPnPDiscoverRequest()
   {
      mHeader = new HttpWebRequestHeader();
      
      // set default header values
      mHeader.setVersion("HTTP/1.1");
      mHeader.setMethod("M-SEARCH");
      mHeader.setPath("*");
      mHeader.setHost(SSDP_MULTICAST_ADDRESS + ":" + UPnP_SSDP_PORT);
      
      // "MAN" header is always "ssdp:discover" (with quotes)
      mHeader.addHeader("MAN", "\"ssdp:discover\"");
      
      // default wait time is 3 seconds
      mHeader.addHeader("MX", "3");
      
      // default search target is all devices
      setSearchTarget("ssdp:all");
   }
   
   /**
    * Gets the HttpWebRequestHeader for this request.
    *
    * @return the HttpWebRequestHeader for this request.
    */
   public HttpWebRequestHeader getHeader()
   {
      return mHeader;
   }
   
   /**
    * Sets the search target to search for.
    * 
    * @param uri the search target (a URI) to search for.
    */
   public void setSearchTarget(String uri)
   {
      getHeader().addHeader("st", uri);
   }
   
   /**
    * Gets the search target to search for.
    * 
    * @return the search target (a URI) to search for.
    */
   public String getSearchTarget()
   {
      return getHeader().getHeaderValue("st");
   }
   
   /**
    * Gets the bytes for this request.
    * 
    * @return the bytes for this request.
    */
   public byte[] getBytes()
   {
      return getHeader().getBytes();
   }
}
