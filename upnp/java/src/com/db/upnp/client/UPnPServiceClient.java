/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client;

import com.db.net.http.HttpWebClient;
import com.db.net.http.HttpWebConnection;
import com.db.net.http.HttpWebRequest;
import com.db.net.http.HttpWebResponse;
import com.db.net.soap.RpcSoapEnvelope;
import com.db.net.soap.SoapOperation;
import com.db.upnp.device.UPnPRootDevice;
import com.db.upnp.service.UPnPService;

/**
 * A UPnPServiceClient is a client that communicates with a UPnPDevice by
 * using its UPnPServices.
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
    * Creates a new UPnPServiceClient for the specified UPnPService.
    * 
    * @param rootDevice the UPnPRootDevice this client is for.
    */
   public UPnPServiceClient(UPnPRootDevice rootDevice)
   {
      // set the root device
      mRootDevice = rootDevice;
   }
   
   /**
    * Sends the passed SoapEnvelope to the passed UPnPService and returns the
    * HTTP response code. The passed envelope will be updated if the server
    * responds with its own envelope.
    * 
    * @param envelope the SoapEnvelope to send.
    * @param service the UPnPService to send the envelope to.
    * 
    * @return true if a connection was made to the service, false if not.
    */
   public boolean sendSoapEnvelope(
      RpcSoapEnvelope envelope, UPnPService service)
   {
      boolean rval = false;
      
      // get the url for the service --
      // use the base URL for the root device
      String url = mRootDevice.getDescription().getBaseUrl();
      if(url.equals(""))
      {
         // use the control URL for the service
         url = service.getControlUrl();
      }
      
      // get an http web connection
      HttpWebClient client = new HttpWebClient();
      HttpWebConnection connection = (HttpWebConnection)client.connect(url);
      if(connection != null)
      {
         rval = true;
         
         // get the xml for the envelope
         String xml = envelope.convertToXml(true, 0, 0);
         
         // get the operation from the envelope
         SoapOperation operation = envelope.getSoapOperation();
         
         // set the soap action
         String soapAction =
            service.getServiceType() + "#" + operation.getName();
         
         // create an HTTP request
         HttpWebRequest request = new HttpWebRequest(connection);
         request.getHeader().setVersion("HTTP/1.1");
         request.getHeader().setMethod("POST");
         request.getHeader().setPath(url + service.getControlUrl());
         request.getHeader().setHost(connection.getRemoteHost());
         request.getHeader().setContentLength(xml.length());
         request.getHeader().setContentType("text/xml; charset=\"utf-8\"");
         request.getHeader().addHeader("SOAPACTION", soapAction);
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
               // receive the body, if any
               if(response.getHeader().getContentLength() > 0)
               {
                  xml = response.receiveBodyString();
                  if(xml != null)
                  {
                     System.out.println("received xml=" + xml);
                     
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
