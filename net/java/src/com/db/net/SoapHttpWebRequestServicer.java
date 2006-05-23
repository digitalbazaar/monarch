/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.util.Date;

import com.db.common.logging.Logger;

/**
 * A soap http web request servicer. Handles soap messages sent via http
 * web requests.
 * 
 * @author Dave Longley
 */
public class SoapHttpWebRequestServicer extends AbstractHttpWebRequestServicer
{
   /**
    * The soap webservice for this servicer.
    */
   protected SoapWebService mSoapWebService;
   
   /**
    * The amount of time to let expire before sending a continue
    * message to the soap client indicating that the soap
    * request hasn't been fulfilled yet (in milliseconds).
    */
   protected static final int CONTINUE_TIME = 10000;
   
   /**
    * Creates a soap http servicer that uses the passed soap web service.
    * 
    * @param sws the soap web service for this servicer.
    */
   public SoapHttpWebRequestServicer(SoapWebService sws) 
   {
      mSoapWebService = sws;
   }   
   
   /**
    * Reads the soap message from an http web request.
    * 
    * @param request the http web request to read the soap message from.
    * @return the soap message read or a soap fault message.
    */
   protected SoapMessage readSoapMessage(HttpWebRequest request)
   {
      SoapMessage sm = null;
      
      getLogger().debug("looking for soap message...");
      
      // receive the body in the request
      byte[] body = request.receiveBody();
      
      // unzip body as appropriate
      if(body != null &&
         request.getHeader().getContentEncoding() != null &&
         request.getHeader().getContentEncoding().contains("gzip"))
      {
         GZipHttpContentCoder coder = new GZipHttpContentCoder();
         
         try
         {
            body = coder.decodeHttpContentData(body);
         }
         catch(Throwable t)
         {
            getLogger().debug(Logger.getStackTrace(t));
         }
      }
      
      // create a soap message
      sm = new SoapMessage();
      sm.setSerializerOptions(SoapMessage.SOAP_REQUEST);
      
      String xml = "";
      if(body != null)
      {
         xml = new String(body);
         getLogger().debug("received soap xml:\n" + xml);
      }
      
      // see if the soap message was valid
      if(body != null && !sm.convertFromXML(xml))
      {
         getLogger().debug("no valid soap message");
         
         // create soap fault
         sm.setFaultCode(SoapMessage.FAULT_CLIENT);
         sm.setFaultString("No valid soap message found.");
         sm.setFaultActor(mSoapWebService.getURI());
      }
      
      return sm;
   }
   
   /**
    * Sends a soap response using the passed http web response object and
    * based on the passed soap message.
    *
    * @param response the http web response to send the soap message with.
    * @param sm the soap message for the response.
    */
   protected void sendSoapResponse(HttpWebResponse response, SoapMessage sm)
   {
      try
      {
         // get the content encoding for the response
         String contentEncoding = response.getHeader().getContentEncoding();
         
         // create a soap service call thread for servicing the remote call
         SoapServiceCallThread t = new SoapServiceCallThread(sm);
         t.start();

         // while waiting for the call thread to complete, keep the
         // connection alive
         long lastTime = new Date().getTime();
         long currentTime = lastTime;
         boolean connected = true;
         while(t.isAlive() && connected)
         {
            if((currentTime - lastTime) >= CONTINUE_TIME)
            {
               // send http continue message, still waiting on soap
               // service to respond
               connected = response.sendContinueResponse();
               
               // reset last time
               lastTime = new Date().getTime();
            }
            
            // try to join the thread
            t.join(1);
            
            // update current time
            currentTime = new Date().getTime();
         }
   
         // send soap response if we are still connected
         if(connected)
         {
            // soap response is ready, convert soap message
            String xml = sm.convertToXML();
            
            byte[] body = xml.getBytes();
            
            // zip body as appropriate
            if(response.getHeader().getContentEncoding() != null &&
               response.getHeader().getContentEncoding().contains("gzip"))
            {
               GZipHttpContentCoder coder = new GZipHttpContentCoder();
               body = coder.encodeHttpContentData(body);
            }
            
            // set header
            response.getHeader().useOKStatusCode();
            response.getHeader().setContentType("text/xml");
            response.getHeader().setContentEncoding(contentEncoding);
            response.getHeader().setContentLength(body.length);
            
            // send header
            if(response.sendHeader())
            {
               // send body
               response.sendBody(body);
            }
         }
      }
      catch(Throwable t)
      {
         // send server error response
         response.sendServerErrorResponse();
         getLogger().debug(Logger.getStackTrace(t));
      }
   }
   
   /**
    * Sends an http web response with the wsdl (Web Service Definition Language)
    * for the associated soap service.
    *
    * @param response the http web response to send the wsdl with.
    * @param wsdl the wsdl to send.
    */
   protected void sendWsdlResponse(HttpWebResponse response, String wsdl)
   {
      getLogger().debug("sending wsdl in response...");
      
      byte[] body = wsdl.getBytes();
      
      // zip body as appropriate
      if(response.getHeader().getContentEncoding() != null &&
         response.getHeader().getContentEncoding().contains("gzip"))
      {
         GZipHttpContentCoder coder = new GZipHttpContentCoder();
         
         try
         {
            body = coder.encodeHttpContentData(body);
         }
         catch(Throwable t)
         {
            getLogger().debug(Logger.getStackTrace(t));
         }
      }
      
      // set header
      response.getHeader().useOKStatusCode();
      response.getHeader().setContentType("text/xml");
      response.getHeader().setContentLength(body.length);
      
      // send header
      if(response.sendHeader())
      {
         // send body
         response.sendBody(body);
      }
   }
   
   /**
    * Services a GET request.
    * 
    * @param request the http web request to service.
    * @param response the http web response to respond with.
    */
   protected void serviceGetRequest(HttpWebRequest request,
                                    HttpWebResponse response)
   {
      if(mSoapWebService.getWsdlPath().equals(request.getHeader().getPath()))
      {
         getLogger().debug("wsdl path match found");
         
         // get the wsdl from the soap web service
         String wsdl = mSoapWebService.getWsdl();
         if(wsdl != null && !wsdl.equals(""))
         {
            // send a wsdl in the response
            sendWsdlResponse(response, wsdl);
         }
         else
         {
            // send not found response
            response.sendNotFoundResponse();
         }
      }
      else
      {
         // send not found response
         response.sendNotFoundResponse();
      }
   }
   
   /**
    * Services a POST request.
    * 
    * @param request the http web request to service.
    * @param response the http web response to respond with.
    */
   protected void servicePostRequest(HttpWebRequest request,
                                     HttpWebResponse response)
   {
      // see if content type is xml
      String contentType = request.getHeader().getContentType();
      if(contentType != null && contentType.indexOf("text/xml") != -1)
      {
         getLogger().debug("http content is text/xml");
         
         // save the content encoding for the response
         String contentEncoding = response.getHeader().getContentEncoding();

         // get a soap message reference
         SoapMessage sm = null;
         
         // get the soap action
         String soapAction = request.getHeader().getHeader("SOAPAction");
         getLogger().debug("SOAPAction is=" + soapAction);
         
         // see if the client is expecting a continue
         String expect = request.getHeader().getHeader("Expect"); 
         if(expect != null && expect.equalsIgnoreCase("100-continue"))
         {
            // check to see if the soap action is appropriate
            if(mSoapWebService.isSoapActionValid(soapAction))
            {
               getLogger().debug("sending 100 continue to get soap message...");
               
               // send the client a continue response
               response.sendContinueResponse();
               
               // read the soap message
               sm = readSoapMessage(request);
            }
            else
            {
               // create a soap fault message
               sm = new SoapMessage();
               sm.setFaultCode(SoapMessage.FAULT_CLIENT);
               sm.setFaultString("Soap action is invalid.");
               sm.setFaultActor(mSoapWebService.getURI());
            }
         }
         else
         {
            // read the soap message
            sm = readSoapMessage(request);
         }
         
         // set the client IP in the soap message
         sm.setClientIP(request.getRemoteIP());
         
         // set content encoding for response
         response.getHeader().setContentEncoding(contentEncoding); 
         
         // send the soap response
         sendSoapResponse(response, sm);
      }
      else
      {
         getLogger().debug("http content is not text/xml,contentType=" +
                           contentType);
         
         // send bad request
         response.sendBadRequestResponse();
      }
   }
   
   /**
    * Services an http web request that uses either a GET or POST method and
    * responds using the passed HttpWebResponse object. If the request
    * method is GET, then a wsdl will be returned for the soap service
    * associated with this servicer. If the method is POST, then a soap
    * method invocation will be attempted.
    * 
    * @param request the http web request.
    * @param response the http web response.
    */
   public void serviceHttpWebRequest(HttpWebRequest request,
                                     HttpWebResponse response)
   {
      // response will always vary
      response.getHeader().addHeader("Vary", "*");
      
      // if the request accepts gzip, set the content-encoding to gzip
      if(request.getHeader().getAcceptEncoding() != null &&
         request.getHeader().getAcceptEncoding().contains("gzip"))
      {
         response.getHeader().setContentEncoding("gzip");
      }
      
      if(request.getHeader().getMethod().equals("GET"))
      {
         serviceGetRequest(request, response);
      }
      else if(request.getHeader().getMethod().equals("POST"))
      {
         servicePostRequest(request, response);
      }
      else
      {
         // send a method not allowed response
         response.sendMethodNotAllowedResponse();
      }
   }
   
   /**
    * A thread that uses the soap service to call a soap method.
    */
   public class SoapServiceCallThread extends Thread
   {
      /**
       * The soap message.
       */
      protected SoapMessage mSoapMessage;
      
      /**
       * Creates a new soap service thread.
       * 
       * @param sm the soap message to use in the soap call.
       */
      public SoapServiceCallThread(SoapMessage sm)
      {
         mSoapMessage = sm;
      }
      
      /**
       * Runs the soap web service's soap method.
       */
      public void run()
      {
         mSoapWebService.callSoapMethod(mSoapMessage);
      }
   }
}
