/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import com.db.net.soap.SoapHttpClient;

/**
 * A UPnPServiceClient is a client that communicates with a UPnPDevice by
 * using one of its UPnPServices.
 * 
 * @author Dave Longley
 */
public class UPnPServiceClient
{
   /**
    * The SOAP client used to communicate with a UPnPDevice.
    */
   protected SoapHttpClient mSoapClient;
   
   /**
    * Creates a new UPnPServiceClient.
    */
   public UPnPServiceClient()
   {
   }
}
