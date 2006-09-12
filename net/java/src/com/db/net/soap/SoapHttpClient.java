/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URL;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.WebConnection;
import com.db.net.http.GZipHttpContentCoder;
import com.db.net.http.HttpBodyPartHeader;
import com.db.net.http.HttpWebClient;
import com.db.net.http.HttpWebConnection;
import com.db.net.http.HttpWebRequest;
import com.db.net.http.HttpWebResponse;
import com.db.net.wsdl.Wsdl;

/**
 * This is the base class for a SOAP client that uses HTTP.
 * 
 * @author Dave Longley
 */
public class SoapHttpClient extends HttpWebClient implements SoapWebClient
{
   /**
    * The version of this soap client.
    */
   public static final String VERSION = "1.0";
   
   /**
    * The path to the Wsdl. 
    */
   protected String mWsdlPath;
   
   /**
    * The Wsdl for the soap web service.
    */
   protected Wsdl mWsdl;

   /**
    * The Wsdl port type to communicate according to.
    */
   protected String mPortType;
   
   /**
    * Whether or not gzip compression should be used.
    */
   protected boolean mUseGZip;
   
   /**
    * Creates a new SoapHttpClient with no specified endpoint address (URL) to
    * connect to.
    * 
    * FUTURE CODE: The "wsdlPath" should become a full path to a WSDL and the
    * URL for using the web service should be obtained from the parsed WSDL
    * and set in this client. If a WSDL is not going to be used, then another
    * constructor should be specified that allows the web service endpoint to
    * be set directly.
    * 
    * @param wsdlPath the path to the wsdl.
    * @param portType the Wsdl port type to communicate according to.
    */
   public SoapHttpClient(String wsdlPath, String portType)
   {
      mWsdlPath = wsdlPath;
      mWsdl = null;
      mPortType = portType;
      
      // do not use gzip compression by default
      useGZip(false);
   }
   
   /**
    * Creates a new SoapHttpClient with the endpoint address (URL)
    * to connect to.
    * 
    * @param endpointAddress the endpoint address to connect to.
    * @param wsdlPath the path to the wsdl.
    * @param portType the Wsdl port type to communicate according to.
    * 
    * @throws MalformedURLException
    */
   public SoapHttpClient(
      String endpointAddress, String wsdlPath, String portType)
   throws MalformedURLException
   {
      this(new URL(endpointAddress), wsdlPath, portType);
   }
   
   /**
    * Creates a new SoapHttpClient with the endpoint address (URL)
    * to connect to.
    * 
    * @param endpointAddress the endpoint address to connect to.
    * @param wsdlPath the path to the wsdl.
    * @param portType the Wsdl port type to communicate according to.
    * 
    * @throws MalformedURLException
    */
   public SoapHttpClient(
      URI endpointAddress, String wsdlPath, String portType)
   throws MalformedURLException
   {
      this(endpointAddress.toURL(), wsdlPath, portType);
   }
   
   /**
    * Creates a new SoapHttpClient with the endpoint address (URL)
    * to connect to.
    * 
    * @param endpointAddress the endpoint address to connect to.
    * @param wsdlPath the path to the wsdl.
    * @param portType the Wsdl port type to communicate according to.
    */
   public SoapHttpClient(
      URL endpointAddress, String wsdlPath, String portType)
   {
      super(endpointAddress);
      
      mWsdlPath = wsdlPath;
      mWsdl = null;
      mPortType = portType;
      
      // do not use gzip compression by default
      useGZip(false);
   }
   
   /**
    * Creates an http web request for sending a soap message over
    * the specified connection.
    * 
    * @param hwc the http web connection to send the http web request over.
    * @param sm the soap message.
    * @param body the soap xml body in bytes.
    * 
    * @return the http web request to use to send the soap message.
    */
   protected HttpWebRequest createSoapHttpWebRequest(
      HttpWebConnection hwc, SoapMessage sm, byte[] body)
   {
      HttpWebRequest request = new HttpWebRequest(hwc);
      
      // create the http web request
      request.getHeader().setMethod("POST");
      request.getHeader().setPath(getWebServicePath());
      request.getHeader().setVersion("HTTP/1.1");
      request.getHeader().setHost(
         getUrl().getHost() + ":" + getUrl().getPort());
      request.getHeader().setContentType("text/xml; charset=utf-8");
      request.getHeader().setContentLength(body.length);
      request.getHeader().setUserAgent("Digital Bazaar SOAP Client " + VERSION);
      request.getHeader().setConnection("close");
      request.getHeader().addHeader(
         "SOAPAction", "\"" + sm.getSoapAction() + "\"");
      
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
      request.getHeader().setHost(
         getUrl().getHost() + ":" + getUrl().getPort());
      request.getHeader().setUserAgent(
         "Digital Bazaar SOAP Client " + VERSION);
      request.getHeader().setConnection("close");

      // accept gzip compression
      request.getHeader().setAcceptEncoding("gzip,deflate");
      
      return request;
   }
   
   /**
    * Gets the soap method result from a soap xml string.
    * 
    * @param sm the soap message used in the soap request.
    * @param xml the soap xml string to get the soap method result from.
    * 
    * @return the soap method result.
    * 
    * @exception SoapFaultException thrown when a soap fault is raised.
    */
   protected Object getSoapMethodResult(SoapMessage sm, String xml)
   throws SoapFaultException
   {
      Object rval = null;
      
      try
      {
         if(sm.isSoapEnvelopeLoggingPermitted())
         {
            getLogger().debugData(getClass(), "received soap xml:\n" + xml);
         }
         
         // create a new soap message for reading the response xml
         sm.setXmlSerializerOptions(SoapMessage.SOAP_RESPONSE);
         if(sm.convertFromXml(xml))
         {
            if(sm.isResponse())
            {
               // get results
               Object[] result = sm.getResults();
               if(result.length > 0)
               {
                  if(sm.isSoapEnvelopeLoggingPermitted())
                  {
                     getLogger().debug(getClass(),
                        "soap message result: " + result[0]);
                  }
                  
                  rval = result[0];
               }
               else
               {
                  getLogger().debug(getClass(),
                     "no soap message result.");
               }
            }
            else if(sm.isFault())
            {
               // throw exception, soap fault
               throw new SoapFaultException(
                  sm, "SOAP Fault: " + sm.getFaultString());
            }
         }
      }
      catch(SoapFaultException sfe)
      {
         // throw the soap fault exception
         throw sfe;
      }
      catch(NullPointerException e)
      {
         // log error
         getLogger().error(getClass(),
            "could not get soap method result!, an exception occurred," +
            "exception= " + e);
         getLogger().debug(getClass(), Logger.getStackTrace(e));

         // create a soap fault
         sm.setFaultCode(SoapMessage.FAULT_SERVER);
         sm.setFaultString(
            "An exception was thrown while processing the soap message " +
            "from the server.");
         sm.setFaultActor("");
         
         // throw a soap fault exception
         throw new SoapFaultException(
            sm, "SOAP Fault: " + sm.getFaultString(), e);
      }

      return rval;
   }
   
   /**
    * Creates a soap request for this soap web client.
    * 
    * @param wsdl the wsdl for the soap web service.
    * @param method the name of the soap method to call.
    * @param params the parameters for the soap method.
    * 
    * @return a soap message for this soap web client.
    */
   public SoapMessage createSoapRequest(
      Wsdl wsdl, String method, Object[] params)
   {
      // create a soap request
      SoapMessage sm = new SoapMessage(wsdl, mPortType);
      sm.setXmlSerializerOptions(SoapMessage.SOAP_REQUEST);
      sm.setMethod(method);
      sm.setParameters(params);

      // FIXME: do we need security extensions for soap web clients?
      sm.setSoapEnvelopeLoggingPermitted(false);
      
      return sm;
   }
   
   /**
    * Calls a remote soap method over the passed web connection using
    * the passed soap request message.
    *
    * @param wc the http web connection to call the soap method over.
    * @param sm the soap request message for the method.
    * 
    * @return the return value from the soap method.
    * 
    * @exception SoapFaultException thrown when a soap fault is raised.
    */
   public Object callSoapMethod(WebConnection wc, SoapMessage sm)
   throws SoapFaultException   
   {
      Object rval = null;
      
      try
      {
         // set the attachment web connection for the soap message
         sm.setAttachmentWebConnection((HttpWebConnection)wc);
         
         // get the xml for the soap message
         String xml = sm.convertToXml();
         byte[] body = xml.getBytes();
         
         // create a soap http web request
         HttpWebRequest request = createSoapHttpWebRequest(
            (HttpWebConnection)wc, sm, body);
         
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

         if(sm.isSoapEnvelopeLoggingPermitted())
         {
            getLogger().debugData(getClass(), "sending soap xml:\n" + xml);
         }
         
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
               
               // get the soap method result
               rval = getSoapMethodResult(sm, xml);
               
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
               
               // create a soap fault
               sm.setFaultCode(SoapMessage.FAULT_SERVER);
               sm.setFaultString(
                  "The response could not be received from the server.");
               sm.setFaultActor(getUrl().toString());
                  
               // throw a soap fault exception
               throw new SoapFaultException(
                  sm, "SOAP Fault: " + sm.getFaultString());
            }
         }
         else
         {
            getLogger().error(getClass(),
               "could not send request to soap server!");
            
            // create a soap fault
            sm.setFaultCode(SoapMessage.FAULT_SERVER);
            sm.setFaultString(
               "The request could not be sent to the server.");
            sm.setFaultActor(getUrl().toString());
               
            // throw a soap fault exception
            throw new SoapFaultException(
               sm, "SOAP Fault: " + sm.getFaultString());
         }
      }
      catch(SoapFaultException sfe)
      {
         // throw the soap fault exception
         throw sfe;
      }
      catch(IOException e)
      {
         // log error
         getLogger().error(getClass(),
            "could not execute soap method!, an exception occurred," +
            "exception= " + e);
         getLogger().debug(getClass(), Logger.getStackTrace(e));
            
         // create a soap fault
         sm.setFaultCode(SoapMessage.FAULT_SERVER);
         sm.setFaultString(
            "An exception was thrown while processing the soap message " +
            "from the server.");
         sm.setFaultActor(getUrl().toString());
            
         // throw a soap fault exception
         throw new SoapFaultException(
            sm, "SOAP Fault: " + sm.getFaultString(), e);         
      }
      catch(NullPointerException e)
      {
         // log error
         getLogger().error(getClass(),
            "could not execute soap method!, an exception occurred," +
            "exception= " + e);
         getLogger().debug(getClass(), Logger.getStackTrace(e));
            
         // create a soap fault
         sm.setFaultCode(SoapMessage.FAULT_SERVER);
         sm.setFaultString(
            "An exception was thrown while processing the soap message " +
            "from the server.");
         sm.setFaultActor(getUrl().toString());
            
         // throw a soap fault exception
         throw new SoapFaultException(
            sm, "SOAP Fault: " + sm.getFaultString(), e);
      }
      
      return rval;
   }
   
   /**
    * Calls a remote soap method. A connection is established to the
    * appropriate soap web service, used to execute the remote method, and
    * then disconnected.
    * 
    * @param method the name of the remote method.
    * @param params the parameters for the remote method.
    * 
    * @return the return value from the remote soap method or null.
    * 
    * @exception SoapFaultException thrown when a SOAP fault is raised.
    */
   public Object callSoapMethod(String method, Object[] params)
   throws SoapFaultException
   {
      Object rval = null;
      
      // get the wsdl
      Wsdl wsdl = getWsdl();
      if(wsdl != null)
      {
         // create a soap request
         SoapMessage sm = createSoapRequest(wsdl, method, params);
         
         // connect to the soap server
         HttpWebConnection hwc = connect();
         if(hwc != null)
         {
            // call the soap method
            rval = callSoapMethod(hwc, sm);
            
            // disconnect web connection
            hwc.disconnect();
            
            getLogger().debug(getClass(),
               "soap web client web connection closed.");
         }
         else
         {
            // log error
            getLogger().error(getClass(),
               "soap web client could not establish connection!");
               
            // create a soap fault
            sm.setFaultCode(SoapMessage.FAULT_SERVER);
            sm.setFaultString(
               "The server could not be reached.");
            sm.setFaultActor(getUrl().toString());
            
            // throw a soap fault exception
            throw new SoapFaultException(
               sm, "SOAP Fault: " + sm.getFaultString());
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the wsdl for a web service. Uses the internally set wsdl url.
    * 
    * @return the wsdl the wsdl was successfully parsed, null otherwise.
    */
   public Wsdl getWsdl()
   {
      if(mWsdl == null)
      {
         mWsdl = getWsdl(mWsdlPath);
      }
      
      return mWsdl;
   }
   
   /**
    * Gets the wsdl for a web service.
    * 
    * @param wsdlUrl the url for the wsdl.
    * 
    * @return the wsdl the wsdl was successfully parsed, null otherwise.
    */
   public synchronized Wsdl getWsdl(String wsdlUrl)
   {
      Wsdl rval = null;

      // remove old wsdl
      mWsdl = null;
      
      long st = System.currentTimeMillis();
      
      try
      {
         // send http get request
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
            String body = null;
            if(receiveResponseHeader(response))
            {
               // receive body
               body = response.receiveBodyString();
            }
            
            // disconnect web connection
            hwc.disconnect();
            
            getLogger().debug(getClass(),
               "soap web client web connection closed.");
            
            String contentType = response.getHeader().getContentType();
            if(body != null && contentType != null &&
               contentType.indexOf("text/xml") != -1)
            {
               // convert the wsdl from xml
               Wsdl wsdl = new Wsdl();
               if(wsdl.convertFromXml(body))
               {
                  rval = wsdl;
                  
                  // save path and wsdl
                  mWsdlPath = wsdlUrl;
                  mWsdl = wsdl;
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
    * Gets the logger for this soap web client.
    * 
    * @return the logger for this soap web client.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
