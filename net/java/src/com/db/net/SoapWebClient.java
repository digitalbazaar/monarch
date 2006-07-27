/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

import java.util.HashMap;
import java.util.Vector;

/**
 * This is the base class for soap web clients.
 * 
 * @author Dave Longley
 */
public class SoapWebClient extends HttpWebClient implements RpcClient
{
   /**
    * The version of this soap web client.
    */
   public static final String VERSION = "1.0";
   
   /**
    * The web service information. 
    */
   protected String mWsdlPath;
   
   /**
    * Wsdl parser.
    */
   protected WsdlParser mWsdlParser;
   
   /**
    * Whether or not gzip compression should be used.
    */
   protected boolean mUseGZip;
   
   /**
    * Creates a new soap web client with the endpoint address to connect to.
    * 
    * @param endpointAddress the endpoint address to connect to.
    * @param wsdlUrl the url to the wsdl.
    */
   public SoapWebClient(String endpointAddress, String wsdlUrl)
   {
      super(endpointAddress);
      
      mWsdlPath = wsdlUrl;
      mWsdlParser = null;
      
      // do not use gzip compression by default
      useGZip(false);
   }
   
   /**
    * Converts the result string from a soap method to the appropriate
    * type of object.
    * 
    * @param result the result string from the soap method.
    * @param resultType the appropriate type of the result.
    * 
    * @return the result converted to the appropriate type.
    */
   protected Object convertResult(String result, Class resultType)
   {
      Object rval = null;
      
      getLogger().debug(getClass(), "result type: " + resultType);
      
      try
      {
         rval = WsdlParser.parseObject(result, resultType);
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), "return value invalid");
      }
      
      return rval;
   }
   
   /**
    * Creates an http web request for sending a soap message over
    * the specified connection.
    * 
    * @param hwc the http web connection to send the http web request over.
    * @param body the soap xml body in bytes.
    * 
    * @return the http web request to use to send the soap message.
    */
   protected HttpWebRequest createSoapHttpWebRequest(HttpWebConnection hwc,
                                                     byte[] body)
   {
      HttpWebRequest request = new HttpWebRequest(hwc);
      
      // create the http web request
      request.getHeader().setMethod("POST");
      request.getHeader().setPath(getWebServicePath());
      request.getHeader().setVersion("HTTP/1.1");
      request.getHeader().setHost(getHost() + ":" + getPort());
      request.getHeader().setContentType("text/xml; charset=utf-8");
      request.getHeader().setContentLength(body.length);
      request.getHeader().setUserAgent("Digital Bazaar SOAP Client " + VERSION);
      request.getHeader().setConnection("close");
      request.getHeader().addHeader("SOAPAction", "\"\"");
      
      // use gzip compression as appropriate
      if(usesGZip())
      {
         request.getHeader().setContentEncoding("gzip");
      }
      
      // accept gzip compression
      request.getHeader().setAcceptEncoding("gzip,deflate");
      
      return request;
   }
   
   /**
    * Creates an http web request for requesting a wsdl.
    * 
    * @param hwc the http web connection to send the http web request over.
    * 
    * @return the http web request to use to request the wsdl.
    */
   protected HttpWebRequest createWsdlHttpWebRequest(HttpWebConnection hwc)
   {
      // create http web request
      HttpWebRequest request = new HttpWebRequest(hwc);
      request.getHeader().setMethod("GET");
      request.getHeader().setPath(mWsdlPath);
      request.getHeader().setVersion("HTTP/1.1");
      request.getHeader().setHost(getHost() + ":" + getPort());
      request.getHeader().setUserAgent("Digital Bazaar SOAP Client " + 
                                       VERSION);
      request.getHeader().setConnection("close");

      // accept gzip compression
      request.getHeader().setAcceptEncoding("gzip,deflate");
      
      return request;
   }
   
   /**
    * Gets the soap method result from a soap xml string.
    * 
    * @param xml the soap xml string to get the soap method result from.
    * @param resultType the type for the result. 
    * 
    * @return the soap method result.
    */
   protected Object getSoapMethodResult(String xml, Class resultType)
   {
      Object rval = null;
      
      try
      {
         // see if xml was acceptable
         if(xml != null)
         {
            // create a new soap message for reading the response xml
            SoapMessage sm = new SoapMessage();
            sm.setSerializerOptions(SoapMessage.SOAP_RESPONSE);
            if(sm.convertFromXml(xml))
            {
               // get result
               String result = sm.getResult();
               getLogger().debug(getClass(), "soap message result: " + result);
         
               // convert soap method result to appropriate return type
               rval = convertResult(result, resultType);
            }
         }
         else
         {
            getLogger().error(getClass(), "could not get soap method result!");
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "could not get soap method result!, an exception occurred," +
            "exception= " + t);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }

      return rval;
   }
   
   /**
    * Creates a soap request message given the passed soap method and
    * parameters.
    * 
    * @param method the soap method for the request message.
    * @param params the parameters for the soap method.
    * @param wsdlParser the wsdl parser to use to create the soap message.
    * 
    * @return the soap request or null if the method was invalid.
    */
   public SoapMessage createSoapRequestMessage(
      String method, Vector params, WsdlParser wsdlParser)
   {
      SoapMessage sm = null;
      
      if(params == null)
      {
         params = new Vector();
      }

      if(wsdlParser != null)
      {
         // get the param map
         HashMap paramMap = wsdlParser.getParamMap(method, params);
         if(paramMap != null)
         {
            // create the soap message
            sm = new SoapMessage();
            sm.setSerializerOptions(SoapMessage.SOAP_REQUEST);
            sm.setMethod(method);
            sm.setParams(paramMap);
            sm.setNamespace(wsdlParser.getNamespace());
         }
      }
      
      return sm;
   }   
   
   /**
    * Calls a remote soap method over the passed http web connection using
    * the passed soap request message.
    *
    * @param hwc the http web connection to call the soap method over.
    * @param sm the soap request message for the method.
    * @param wsdlParser the wsdl parser to use.
    * 
    * @return the return value from the soap method.
    */
   public Object callSoapMethod(
      HttpWebConnection hwc, SoapMessage sm, WsdlParser wsdlParser)
   {
      Object rval = null;
      
      try
      {
         // set the attachment web connection for the soap message
         sm.setAttachmentWebConnection(hwc);
         
         // get the xml for the soap message
         String xml = sm.convertToXml();
         byte[] body = xml.getBytes();
         
         // create a soap http web request
         HttpWebRequest request = createSoapHttpWebRequest(hwc, body);
         
         // get an http web response
         HttpWebResponse response = request.createHttpWebResponse();
         
         // zip body as appropriate
         if(request.getHeader().getContentEncoding() != null &&
            request.getHeader().getContentEncoding().contains("gzip"))
         {
            GZipHttpContentCoder coder = new GZipHttpContentCoder();
            body = coder.encodeHttpContentData(body);
            request.getHeader().setContentLength(body.length);
         }
         
         getLogger().debug(getClass(), "sending soap xml:\n" + xml);
         
         // start soap method timer
         long st = System.currentTimeMillis();
         
         // send the request to the server
         if(sendRequest(request, body))
         {
            // receive response header from the soap server
            if(receiveResponseHeader(response))
            {
               body = null;
               
               // see if the response is multipart or not
               if(response.isMultipart())
               {
                  // set the response header for the soap message
                  sm.setHttpHeader(response.getHeader());
                  
                  // get the first part header
                  HttpBodyPartHeader header =
                     response.receiveBodyPartHeader();
                  
                  // receive the body part body if the header was received
                  if(header != null)
                  {
                     body = response.receiveBodyPartBody(header);
                  }
               }
               else
               {
                  // receive the body of the request
                  body = response.receiveBody();
               }

               xml = "";
               if(body != null)
               {
                  xml = new String(body); 
               }
               
               // get soap method result type
               Class resultType = wsdlParser.getReturnType(sm.getMethod());
               
               // get the soap method result
               rval = getSoapMethodResult(xml, resultType);
               
               // get end time
               long et = System.currentTimeMillis();
               long timespan = et - st;
               getLogger().debug(getClass(),
                  "total soap method (" + sm.getMethod() + ") " +
                  "time: " + timespan + " ms");
            }
            else
            {
               getLogger().error(getClass(),
                  "could not receive response from soap server!");
            }
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "could not execute soap method!, an exception occurred," +
            "exception= " + t);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Calls a remote method. A connection is established, used to execute
    * the remote method, and then disconnected.
    * 
    * @param method the name of the remote method.
    * @param params the parameters for the remote method.
    * 
    * @return the return value from the remote method or null. 
    */
   public Object callRemoteMethod(String method, Vector params)
   {
      Object rval = null;
      
      // get wsdl parser
      WsdlParser wsdlParser = getWsdlParser();
      if(wsdlParser != null)
      {
         // create a soap request message
         SoapMessage sm = createSoapRequestMessage(method, params, wsdlParser);
         if(sm != null)
         {
            // connect to the soap server
            HttpWebConnection hwc = connect();
            if(hwc != null)
            {
               try
               {
                  // call the soap method
                  rval = callSoapMethod(hwc, sm, wsdlParser);
                  
                  // disconnect web connection
                  hwc.disconnect();
                  
                  getLogger().debug(getClass(),
                     "soap web client web connection closed.");
               }
               catch(Throwable t)
               {
                  getLogger().error(getClass(),
                     "could not execute soap method!, an exception occurred," +
                     "exception= " + t);
                  getLogger().debug(getClass(), Logger.getStackTrace(t));
               }
            }
            else
            {
               getLogger().error(getClass(),
                  "soap web client could not establish connection!");
            }
         }
         else
         {
            getLogger().error(getClass(),
               "could not call remote method, " +
               "soap method invalid: \"" + method + "\"");
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the wsdl parser for a web service. Uses the internally set wsdl url.
    * 
    * @return the wsdl parser the wsdl was successfully parsed, null otherwise.
    */
   public WsdlParser getWsdlParser()
   {
      if(mWsdlParser == null)
      {
         mWsdlParser = getWsdlParser(mWsdlPath);
      }
      
      return mWsdlParser;
   }
   
   /**
    * Gets the wsdl parser for a web service.
    * 
    * @param wsdlUrl the url for the wsdl.
    * 
    * @return the wsdl parser the wsdl was successfully parsed, null otherwise.
    */
   public synchronized WsdlParser getWsdlParser(String wsdlUrl)
   {
      WsdlParser rval = null;

      // remove old wsdl parser
      mWsdlParser = null;
      
      long st = System.currentTimeMillis();
      
      // sent http get request
      try
      {
         HttpWebConnection hwc = connect();
         if(hwc != null)
         {
            // create http web request
            HttpWebRequest request = createWsdlHttpWebRequest(hwc);
            
            // send request for wsdl
            sendRequest(request);
            
            // create http web response
            HttpWebResponse response = request.createHttpWebResponse();
            
            // receive response header from the soap server
            byte[] body = null;
            if(receiveResponseHeader(response))
            {
               // receive body
               body = response.receiveBody();
            }
            
            // disconnect web connection
            hwc.disconnect();
            
            getLogger().debug(getClass(),
               "soap web client web connection closed.");
            
            String contentType = response.getHeader().getContentType();
            if(body != null &&
               contentType != null && contentType.indexOf("text/xml") != -1)
            {
               WsdlParser wsdlParser = new WsdlParser();
               if(wsdlParser.parseWsdl(new String(body)))
               {
                  rval = wsdlParser;
                  
                  // save parser and path
                  mWsdlParser = wsdlParser;
                  mWsdlPath = wsdlUrl;
               }
            }
         }
         
         if(rval != null)
         {
            long et = System.currentTimeMillis();
            long timespan = et - st;
            getLogger().debug(getClass(),
               "wsdl retrieved in " + timespan + " ms");
         }
         else
         {
            getLogger().error(getClass(),
               "could not convert received wsdl from xml!");
         }
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), LoggerManager.getStackTrace(t));
      }
      
      return rval;
   }

   /**
    * Sets the endpoint address and wsdl url.
    * 
    * @param endpointAddress the endpoint address for this client.
    * @param wsdlUrl the wsdl url.
    */
   public synchronized void setEndpointAddress(
      String endpointAddress, String wsdlUrl)
   {
      if(!mEndpointAddress.equals(endpointAddress))
      {
         mEndpointAddress = endpointAddress;
         parseEndpointAddress();
      }
      
      if(!mWsdlPath.equalsIgnoreCase(wsdlUrl))
      {
         mWsdlPath = wsdlUrl;
         mWsdlParser = null;
      }
   }

   /**
    * Sets whether or not gzip compression should be used when sending
    * soap requests.
    * 
    * @param useGZip true if gzip compression should be used when sending
    *                soap requests, false if not. 
    */
   public void useGZip(boolean useGZip)
   {
      mUseGZip = useGZip;
   }
   
   /**
    * Gets whether or not gzip compression is used when sending soap requests.
    * 
    * @return true if gzip compression is used when sending soap requests,
    *         false if not. 
    */
   public boolean usesGZip()
   {
      return mUseGZip;
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
