/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.upnp;

import com.db.net.soap.SoapWebClient;

/**
 * A UPnP Control Point. This is a single point that is capable of
 * controlling a single UPnP device via its SOAP interface.
 * 
 * @author Dave Longley
 */
public class UPnPControlPoint
{
   /**
    * The SOAP client used to communicate with the UPnP device.
    */
   protected SoapWebClient mSoapClient;
   
   /**
    * Creates a new UPnPControlPoint.
    */
   public UPnPControlPoint()
   {
   }
}
