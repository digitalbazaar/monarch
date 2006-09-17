/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client;

import java.net.MalformedURLException;

import com.db.net.http.HttpWebClient;
import com.db.upnp.device.UPnPRootDevice;
import com.db.upnp.service.UPnPService;

/**
 * A UPnPServiceClient is a client that communicates with a UPnPDevice by
 * using one of its UPnPServices.
 * 
 * @author Dave Longley
 */
public class UPnPServiceClient
{
   /**
    * The UPnPRootDevice this client is for.
    */
   protected UPnPRootDevice mRootDevice;
   
   /**
    * UPnPService the service this client communicates with.
    */
   protected UPnPService mService;
   
   /**
    * The HTTP client used to make use of a UPnPService.
    */
   protected HttpWebClient mHttpClient;
   
   /**
    * Creates a new UPnPServiceClient for the specified UPnPService.
    * 
    * @param rootDevice the UPnPRootDevice this client is for.
    * @param service the service to communicate with.
    * 
    * @exception MalformedURLException thrown if the base URL for the root
    *                                  device or the control URL for the
    *                                  service are malformed.
    */
   public UPnPServiceClient(UPnPRootDevice rootDevice, UPnPService service)
   throws MalformedURLException
   {
      // set the root device and service
      mRootDevice = rootDevice;
      mService = service;
      
      // create the http client
      mHttpClient = new HttpWebClient();
      
      // get the base URL for the root device
      String baseUrl = rootDevice.getDescription().getBaseUrl();
      
      // get the control URL for the service
      String controlUrl = service.getControlUrl();
      
      // set the URL for the http client
      mHttpClient.setUrl(baseUrl + controlUrl);
   }
}
