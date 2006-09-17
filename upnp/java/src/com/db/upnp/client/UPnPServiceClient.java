/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client;

import java.net.MalformedURLException;

import com.db.net.http.HttpWebClient;
import com.db.net.http.HttpWebConnection;
import com.db.net.http.HttpWebRequest;
import com.db.net.http.HttpWebResponse;
import com.db.net.soap.SoapEnvelope;
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
   
   /**
    * Sends the passed SoapEnvelope to the service and returns the HTTP
    * response code. The passed envelope will be updated if the server responds
    * with its own envelope.
    * 
    * @param envelope the SoapEnvelope to send.
    * 
    * @return the HTTP response code.
    */
   public String sendSoapEnvelope(SoapEnvelope envelope) 
   {
      String rval = "503 Service Unavailable";
      
      // get an http web connection
      HttpWebConnection connection = mHttpClient.connect();
      if(connection != null)
      {
         // get the xml for the envelope
         String xml = envelope.convertToXml(true, 0, 0);
         
         // create an HTTP request
         HttpWebRequest request = new HttpWebRequest(connection);
         request.getHeader().setContentLength(xml.length());
         request.getHeader().setConnection("close");
         
         // send the request header
         if(request.sendHeader())
         {
            // send the request body
            request.sendBody(xml);
            
            // create a response
            HttpWebResponse response = request.createHttpWebResponse();
            
            // receive the response header
            if(response.receiveHeader())
            {
               // set the status code to the return value
               rval = response.getHeader().getStatusCode();
               
               // receive the body, if any
               if(response.getHeader().getContentLength() > 0)
               {
                  xml = response.receiveBodyString();
                  if(xml != null)
                  {
                     // convert the rpc soap envelope from the received xml
                     envelope.convertFromXml(xml);
                     
                  }
               }
            }
         }
         
         // disconnect the connection
         connection.disconnect();
      }
      
      return rval;
   }
}
