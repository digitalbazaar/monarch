/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import java.util.Map;

import com.db.logging.Logger;
import com.db.net.http.AbstractHttpWebRequestServicer;
import com.db.net.http.GzipHttpContentCoder;
import com.db.net.http.HttpBodyPartHeader;
import com.db.net.http.HttpWebRequest;
import com.db.net.http.HttpWebResponse;
import com.db.net.wsdl.Wsdl;

/**
 * A soap http web request servicer. Handles soap messages sent via http
 * web requests.
 * 
 * FUTURE CODE: When we move over to SOAP 1.2 or (even before that if we
 * add complex object support) we want a clean redesign of the soap classes
 * and interfaces to make it more streamlined and easy to use. This may
 * include created a new interface for serializing complex objects to xml.  
 * 
 * @author Dave Longley
 */
public class SoapHttpWebRequestServicer extends AbstractHttpWebRequestServicer
{
   /**
    * The soap web service for this servicer.
    */
   protected SoapWebService mSoapWebService;
   
   /**
    * The amount of time to let expire before sending a continue
    * message to the soap client indicating that the soap
    * request hasn't been fulfilled yet (in milliseconds).
    */
   protected static final int CONTINUE_TIME = 20000;
   
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
    * Reads the rpc soap message from an http web request.
    * 
    * @param request the http web request to read the soap message from.
    * 
    * @return the soap message read or a soap fault message.
    */
   protected RpcSoapMessage readSoapMessage(HttpWebRequest request)
   {
      RpcSoapMessage sm = null;
      
      getLogger().detail(getClass(), "looking for soap message...");
      
      // create a soap message
      sm = mSoapWebService.createSoapMessage();
      
      // see if the request is multipart or not
      byte[] body = null;
      if(request.isMultipart())
      {
         // set the header and attachment connection for the soap message
         sm.setHttpHeader(request.getHeader());
         sm.setAttachmentWebConnection(request.getHttpWebConnection());
         
         // get the first part header
         HttpBodyPartHeader header = request.receiveBodyPartHeader();
         
         // receive the body part body if the header was received
         if(header != null)
         {
            body = request.receiveBodyPartBody(header);
         }
      }
      else
      {
         // receive the body in the request
         body = request.receiveBody();
      }
      
      if(body != null)
      {
         getLogger().debug(getClass(),
            "received soap xml (see debug-data log for the actual xml).");
         
         if(mSoapWebService instanceof SecureSoapWebService)
         {
            SecureSoapWebService secure =
               (SecureSoapWebService)mSoapWebService;
            SoapPermission permission = new SoapPermission("envelope.log");
            if(secure.checkSoapPermission(permission))
            {
               try
               {
                  getLogger().debugData(getClass(),
                     "received soap xml:\n" + new String(body, "UTF-8"));            
               }
               catch(Exception e)
               {
                  // shouldn't happen, UTF-8 is supported
               }
            }
         }
         else
         {
            getLogger().debugData(getClass(), "received soap xml:\n" + body);            
         }
         
         try
         {
            // see if the soap message was valid
            sm.getRpcSoapEnvelope().convertFromXml(new String(body, "UTF-8"));
         }
         catch(Exception e)
         {
            // create soap fault
            SoapFault fault = new SoapFault();
            fault.setFaultCode(SoapFault.FAULT_CLIENT);
            fault.setFaultString("No valid soap message found");
            fault.setFaultActor(mSoapWebService.getURI());
            
            sm.getRpcSoapEnvelope().setSoapFault(fault);
         }
         
         if(!sm.getRpcSoapEnvelope().containsSoapOperation())
         {
            getLogger().debug(getClass(),
               "NO valid soap message found! Sending soap fault.");
            
            // create soap fault
            SoapFault fault = new SoapFault();
            fault.setFaultCode(SoapFault.FAULT_CLIENT);
            fault.setFaultString("No valid soap message found.");
            fault.setFaultActor(mSoapWebService.getURI());

            sm.getRpcSoapEnvelope().setSoapFault(fault);
         }
      }
      else
      {
         getLogger().debug(getClass(),
            "NO valid soap message found! Sending soap fault.");
      
         // create soap fault
         SoapFault fault = new SoapFault();
         fault.setFaultCode(SoapFault.FAULT_CLIENT);
         fault.setFaultString("No valid soap message found.");
         fault.setFaultActor(mSoapWebService.getURI());

         sm.getRpcSoapEnvelope().setSoapFault(fault);
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
   protected void sendSoapResponse(HttpWebResponse response, RpcSoapMessage sm)
   {
      try
      {
         // store the headers for the response
         Map<String, String> headers = response.getHeader().getHeaders();
         
         // create a soap service call thread for servicing the remote call
         SoapServiceCallThread t = new SoapServiceCallThread(sm);
         t.start();

         // while waiting for the call thread to complete, keep the
         // connection alive
         long lastTime = System.currentTimeMillis();
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
               lastTime = System.currentTimeMillis();
            }
            
            // try to join the thread
            t.join(1);
            
            // update current time
            currentTime = System.currentTimeMillis();
         }
   
         // send soap response if we are still connected
         if(connected)
         {
            // soap response is ready, convert soap envelope
            String xml = sm.getSoapEnvelope().convertToXml(true, 0, 0);
            byte[] body = xml.getBytes();
            
            getLogger().debug(getClass(),
               "sending soap xml (see debug-data log for the actual xml).");
            
            if(mSoapWebService instanceof SecureSoapWebService)
            {
               SecureSoapWebService secure =
                  (SecureSoapWebService)mSoapWebService;
               SoapPermission permission = new SoapPermission("envelope.log");
               if(secure.checkSoapPermission(permission))
               {
                  getLogger().debugData(getClass(),
                     "sending soap xml:\n" + xml);            
               }
            }
            else
            {
               getLogger().debugData(getClass(), "sending soap xml:\n" + xml);            
            }
            
            // restore the headers for the response
            response.getHeader().setHeaders(headers);
            
            // zip body as appropriate
            if(response.getHeader().getContentEncoding() != null &&
               response.getHeader().getContentEncoding().contains("gzip"))
            {
               GzipHttpContentCoder coder = new GzipHttpContentCoder();
               body = coder.encodeHttpContentData(body);
            }
            
            // set appropriate headers
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
      }
      catch(Throwable t)
      {
         // send server error response
         response.sendServerErrorResponse();
         getLogger().debug(getClass(), Logger.getStackTrace(t));
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
      getLogger().debug(getClass(), "sending wsdl in response...");
      
      byte[] body = wsdl.getBytes();
      
      // zip body as appropriate
      if(response.getHeader().getContentEncoding() != null &&
         response.getHeader().getContentEncoding().contains("gzip"))
      {
         GzipHttpContentCoder coder = new GzipHttpContentCoder();
         
         try
         {
            body = coder.encodeHttpContentData(body);
         }
         catch(Throwable t)
         {
            getLogger().debug(getClass(), Logger.getStackTrace(t));
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
   protected void serviceGetRequest(
      HttpWebRequest request, HttpWebResponse response)
   {
      if(mSoapWebService.getWsdlPath().equals(request.getHeader().getPath()))
      {
         getLogger().detail(getClass(), "wsdl path match found");
         
         // get the wsdl from the soap web service
         Wsdl wsdl = mSoapWebService.getWsdl();
         if(wsdl != null)
         {
            String xml = wsdl.convertToXml();
            if(!xml.equals(""))
            {
               // send a wsdl xml in the response
               sendWsdlResponse(response, xml);
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
   protected void servicePostRequest(
      HttpWebRequest request, HttpWebResponse response)
   {
      // see if content type is xml
      String contentType = request.getHeader().getContentType();
      if(contentType != null &&
         (contentType.indexOf("text/xml") != -1 ||
         contentType.indexOf("soap") != -1))
      {
         getLogger().detail(getClass(), "http content is text/xml or soap");
         
         // save the content encoding for the response
         String contentEncoding = response.getHeader().getContentEncoding();

         // get a soap message reference
         RpcSoapMessage sm = null;
         
         // get the soap action
         String soapAction = request.getHeader().getHeaderValue("SOAPAction");
         getLogger().detail(getClass(), "SOAPAction is=" + soapAction);
         
         // check to see if the soap action is appropriate
         if(mSoapWebService.isSoapActionValid(soapAction))
         {
            // see if the client is expecting a continue
            String expect = request.getHeader().getHeaderValue("Expect"); 
            if(expect != null && expect.equalsIgnoreCase("100-continue"))
            {
               getLogger().debug(getClass(),
                  "sending 100 continue to get soap message...");
               
               // send the client a continue response
               response.sendContinueResponse();
            }
            
            // read the soap message
            sm = readSoapMessage(request);
         }
         else
         {
            // create a soap fault message
            sm = mSoapWebService.createSoapMessage();
            SoapFault fault = new SoapFault();
            fault.setFaultCode(SoapFault.FAULT_CLIENT);
            fault.setFaultString("Soap action is invalid.");
            fault.setFaultActor(mSoapWebService.getURI());
            
            sm.getRpcSoapEnvelope().setSoapFault(fault);
         }
         
         // set the remote IP in the soap message
         sm.setRemoteIP(request.getRemoteIP());
         
         // set content encoding for response
         response.getHeader().setContentEncoding(contentEncoding); 
         
         // send the soap response
         sendSoapResponse(response, sm);
      }
      else
      {
         getLogger().debug(getClass(), 
            "http content is NOT text/xml -- sending bad request response" +
            ",contentType=" + contentType);
         
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
   @Override
   public void serviceHttpWebRequest(
      HttpWebRequest request, HttpWebResponse response)
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
      protected RpcSoapMessage mSoapMessage;
      
      /**
       * Creates a new soap service thread.
       * 
       * @param sm the soap message to use in the soap call.
       */
      public SoapServiceCallThread(RpcSoapMessage sm)
      {
         mSoapMessage = sm;
      }
      
      /**
       * Runs the soap web service's soap method.
       */
      @Override
      public void run()
      {
         try
         {
            mSoapWebService.callSoapMethod(mSoapMessage);
         }
         catch(Throwable t)
         {
            // raise a soap fault
            SoapFault fault = new SoapFault();
            fault.setFaultCode(SoapFault.FAULT_SERVER);
            fault.setFaultString(
               "An exception was thrown by the server " +
               "when calling the specified soap method.");
            fault.setFaultActor(mSoapWebService.getURI());
            
            mSoapMessage.getRpcSoapEnvelope().setSoapFault(fault);
            
            getLogger().debug(getClass(), 
               "failed to call soap method, sending soap fault" +
               ",reason=" + fault.getFaultString());

            getLogger().debug(getClass(), Logger.getStackTrace(t));
         }
      }
   }
}
