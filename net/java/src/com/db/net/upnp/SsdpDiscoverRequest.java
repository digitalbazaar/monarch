/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.upnp;

import com.db.net.http.HttpWebRequestHeader;

/**
 * An SsdpDiscoverRequest is an HTTP based request that uses the
 * Simple Service Discovery Protocol (SSDP). It is used to discover devices.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/draft_cai_ssdp_v1_03.txt
 * 
 * -----------------------------------------------------------------------
 * For brevity's sake a HTTP SEARCH method enhanced with the 
 * ssdp:discover functionality will be referred to as a ssdp:discover 
 * request.
 * 
 * ssdp:discover requests MUST contain a ST header. ssdp:discover 
 * requests MAY contain a body but the body MAY be ignored if not 
 * understood by the HTTP service.
 * 
 * The ST header contains a single URI. SSDP clients may use the ST 
 * header to specify the service type they want to discover.
 * 
 * This specification only specifies the use of ssdp:discover requests 
 * over HTTP Multicast UDP although it is expected that future 
 * specifications will expand the definition to handle ssdp:discover 
 * requests sent over HTTP TCP.
 * 
 * ssdp:discover requests sent to the SSDP multicast channel/port MUST 
 * have a request-URI of "*". Note that future specifications may allow 
 * for other request-URIs to be used so implementations based on this 
 * specification MUST be ready to ignore ssdp:discover requests on the 
 * SSDP multicast channel/port with a request-URI other than "*".
 *  
 * Only SSDP services that have a service type that matches the value 
 * in the ST header MAY respond to a ssdp:discover request on the SSDP 
 * multicast channel/port.
 * 
 * Responses to ssdp:discover requests sent over the SSDP multicast 
 * channel/port are to be sent to the IP address/port the ssdp:discover 
 * request came from.
 * 
 * A response to a ssdp:discover request SHOULD include the service's 
 * location expressed through the Location and/or AL header. A 
 * successful response to a ssdp:discover request MUST also include the 
 * ST and USN headers.
 * 
 * Response to ssdp:discover requests SHOULD contain a cache-control: 
 * max-age or Expires header. If both are present then they are to be 
 * processed in the order specified by HTTP/1.1, that is, the cache-
 * control header takes precedence of the Expires header. If neither 
 * the cache-control nor the Expires header is provided on the response 
 * to a ssdp:discover request then the information contained in that 
 * response MUST NOT be cached by SSDP clients.
 * 
 * 4.2.1.1.  Example 
 * 
 * M-SEARCH * HTTP/1.1
 * S: uuid:ijklmnop-7dec-11d0-a765-00a0c91e6bf6
 * Host: 239.255.255.250:reservedSSDPport
 * Man: "ssdp:discover"
 * ST: ge:fridge
 * MX: 3
 * -----------------------------------------------------------------------
 * 
 * @author Dave Longley
 */
public class SsdpDiscoverRequest
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
    * Creates a new SsdpDiscoverRequest.
    */
   public SsdpDiscoverRequest()
   {
      mHeader = new HttpWebRequestHeader();
      
      // set default header values
      mHeader.setVersion("HTTP/1.1");
      mHeader.setMethod("M-SEARCH");
      mHeader.setPath("*");
      mHeader.setHost(SSDP_MULTICAST_ADDRESS + ":" + UPnP_SSDP_PORT);
      
      // "MAN" header is always "ssdp:discover" (with quotes)
      mHeader.addHeader("MAN", "\"ssdp:discover\"");
      
      // default "MX" header is 3
      mHeader.addHeader("MX", "3");
      
      // set default "ST" (service type) to ssdp:all
      setServiceType("ssdp:all");
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
    * Sets the service type (header "ST") to search for.
    * 
    * @param uri the service type (a URI) to search for.
    */
   public void setServiceType(String uri)
   {
      getHeader().addHeader("st", uri);
   }
   
   /**
    * Gets the service type (header "ST") to search for.
    * 
    * @return the service type (a URI) to search for.
    */
   public String getServiceType()
   {
      return getHeader().getHeader("st");
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
