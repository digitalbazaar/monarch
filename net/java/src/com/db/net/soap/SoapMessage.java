/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.Iterator;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.http.HttpBodyPartHeader;
import com.db.net.http.HttpHeader;
import com.db.net.http.HttpWebConnection;
import com.db.net.wsdl.Wsdl;
import com.db.net.wsdl.WsdlMessage;
import com.db.net.wsdl.WsdlMessagePart;
import com.db.net.wsdl.WsdlPortType;
import com.db.net.wsdl.WsdlPortTypeOperation;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * This class represents a SOAP message.
 * 
 * FUTURE CODE: We want to split some of the functionality for this
 * message up into separate classes (i.e. request/result/fault). The
 * fault implementation is also incomplete, lacking code to handle
 * a fault detail if one exists. 
 * 
 * @author Dave Longley
 */
public class SoapMessage extends AbstractXmlSerializer
{
   /**
    * The xml schema for a soap message (the xsd).
    */
   public static final String XSD_NAMESPACE_URI =
      "http://www.w3.org/2001/XMLSchema";
   
   /**
    * The xml schema instance for a soap message.
    */
   public static final String XSI_NAMESPACE_URI =
      "http://www.w3.org/2001/XMLSchema-instance";

   /**
    * The soap namespace.
    */
   public static final String SOAP_NAMESPACE_URI =
      "http://schemas.xmlsoap.org/wsdl/soap/";

   /**
    * The encoding schema.
    */
   public static final String ENCODING_NAMESPACE_URI =
      "http://schemas.xmlsoap.org/soap/encoding/";

   /**
    * The envelope schema.
    */
   public static final String ENVELOPE_NAMESPACE_URI =
      "http://schemas.xmlsoap.org/soap/envelope/";
   
   /**
    * The WSDL for the web service.
    */
   protected Wsdl mWsdl;
   
   /**
    * The port type for the web service.
    */
   protected WsdlPortType mPortType;
   
   /**
    * The port type operation for the method.
    */
   protected WsdlPortTypeOperation mPortTypeOperation;
   
   /**
    * The name of the method to execute.
    */
   protected String mMethod;
   
   /**
    * The parameters for the method to execute.
    */
   protected Object[] mParams;
   
   /**
    * The results of the executed method.
    */
   protected Object[] mResults;

   /**
    * The address of the client for this soap message.
    */
   protected String mClientIP;
   
   /**
    * The fault code for this soap message, if any. A fault code
    * can be any of the following:
    * 
    * VersionMismatch.*
    * MustUnderstand.*
    * Client.*
    * Server.*
    */
   protected String mFaultCode;
   
   /**
    * The fault string for this soap message, if any. This is a human
    * readable string that provides a short explanation for the fault.
    */
   protected String mFaultString;
   
   /**
    * The fault actor for this soap message, if any. The fault actor specifies
    * who caused the fault to happen within the soap message path. The fault
    * actor is a URI identifying the source of the fault.
    */
   protected String mFaultActor;
   
   /**
    * An http web connection for reading or writing attachments.
    */
   protected HttpWebConnection mAttachmentWebConnection;
   
   /**
    * The http header for this soap message, if any.
    */
   protected HttpHeader mHttpHeader;
   
   /**
    * A soap fault: when a version mismatch occurs.
    */
   protected final static String FAULT_VERSION_MISMATCH = "VersionMismatch";
   
   /**
    * A soap fault: when something must be understood but is not.
    */
   protected final static String FAULT_MUST_UNDERSTAND = "MustUnderstand";
   
   /**
    * A soap fault: when a soap client faults.
    */
   protected final static String FAULT_CLIENT = "Client";
   
   /**
    * A soap fault: when a soap server faults.
    */
   protected final static String FAULT_SERVER = "Server";
   
   /**
    * XML serializer options.
    */
   protected int mXmlOptions;
   
   /**
    * An XML serializer option for converting to/from a soap request. 
    */
   protected final static int SOAP_REQUEST = 1;
   
   /**
    * An XML serializer option for converting to/from a soap response.
    */
   protected final static int SOAP_RESPONSE = 2;
   
   /**
    * An XML serializer option for converting to/from a soap fault. 
    */
   protected final static int SOAP_FAULT = 4;
   
   /**
    * Creates a new soap message.
    * 
    * @param wsdl the wsdl for the web service this soap message will be used
    *             with.
    * @param portType the name of the port type to use.
    */
   public SoapMessage(Wsdl wsdl, String portType)
   {
      // store the wsdl and port type
      mWsdl = wsdl;
      mPortType = mWsdl.getPortTypes().getPortType(portType);
      mPortTypeOperation = null;
      
      if(mPortType == null)
      {
         throw new IllegalArgumentException(
            "Port Type not valid for given Wsdl!");
      }

      // default to a soap request
      setXmlSerializerOptions(SOAP_REQUEST);
      
      // set default client IP address
      mClientIP = "0.0.0.0";
      
      // set default fault string and actor to blank
      setFaultString("");
      setFaultActor("");
   }
   
   /**
    * Finds the port type operation for the given method name.
    *
    * @param method the method name.
    * 
    * @exception IllegalArgumentException thrown if there is no operation for
    *                                     the given method name.
    */
   protected void findPortTypeOperation(String method)
   throws IllegalArgumentException
   {
      // get the port type operation
      mPortTypeOperation = mPortType.getOperations().getOperation(method);
      
      if(mPortTypeOperation == null)
      {
         throw new IllegalArgumentException(
            "Method is not valid for the given Wsdl Port Type!" +
            ",method=" + method);
      }
   }
   
   /**
    * Gets the WsdlMessage for a SOAP request.
    * 
    * @return the WsdlMessage for a SOAP request.
    */
   protected WsdlMessage getRequestMessage()
   {
      WsdlMessage rval = null;
      
      // get the message name
      String messageName = "";
      if(getPortTypeOperation().usesOnlyInputMessage())
      {
         messageName = getPortTypeOperation().getInputMessageName();
      }
      else if(getPortTypeOperation().usesOnlyOutputMessage())
      {
         messageName = getPortTypeOperation().getOutputMessageName();
      }
      else
      {
         if(getPortTypeOperation().isInputFirst())
         {
            messageName = getPortTypeOperation().getInputMessageName();
         }
         else
         {
            messageName = getPortTypeOperation().getOutputMessageName();
         }
      }
      
      // get the wsdl message
      rval = mWsdl.getMessages().getMessage(messageName);
      
      return rval;
   }
   
   /**
    * Gets the WsdlMessage for a SOAP response.
    * 
    * @return the WsdlMessage for a SOAP response.
    */
   protected WsdlMessage getResponseMessage()
   {
      WsdlMessage rval = null;
      
      // get the message name
      String messageName = "";
      if(getPortTypeOperation().usesOnlyInputMessage())
      {
         messageName = getPortTypeOperation().getInputMessageName();
      }
      else if(getPortTypeOperation().usesOnlyOutputMessage())
      {
         messageName = getPortTypeOperation().getOutputMessageName();
      }
      else
      {
         if(getPortTypeOperation().isInputFirst())
         {
            messageName = getPortTypeOperation().getOutputMessageName();
         }
         else
         {
            messageName = getPortTypeOperation().getInputMessageName();
         }
      }
      
      // get the wsdl message
      rval = mWsdl.getMessages().getMessage(messageName);
      
      return rval;
   }
   
   /**
    * Converts a soap fault from an XmlElement.
    * 
    * @param element the XmlElement to convert from.
    */
   protected void convertSoapFaultFromXmlElement(XmlElement element)
   {
      // get the fault code element
      XmlElement faultcodeElement = element.getFirstChild("faultcode");
      if(faultcodeElement != null)
      {
         // FIXME: this is a hack to remove any namespace prefix
         String[] split = faultcodeElement.getValue().split(":");
         if(split.length > 1)
         {
            setFaultCode(split[1]);
         }
         else
         {
            setFaultCode(faultcodeElement.getValue());
         }
      }
      
      // get the fault string element
      XmlElement faultstringElement = element.getFirstChild("faultcode");
      if(faultstringElement != null)
      {
         setFaultString(faultstringElement.getValue());
      }
      
      // get the fault actor element
      XmlElement faultactorElement = element.getFirstChild("faultcode");
      if(faultactorElement != null)
      {
         setFaultActor(faultactorElement.getValue());
      }
   }
   
   /**
    * Converts a soap request from an XmlElement.
    * 
    * @param element the XmlElement to convert from.
    */
   protected void convertSoapRequestFromXmlElement(XmlElement element)
   {
      // set the method name to the basic element name
      setMethod(XmlElement.getBasicName(element.getName()));
      
      // get the request message
      WsdlMessage message = getRequestMessage();
      
      // build a parameters array
      Object[] params = new Object[message.getParts().size()];
      if(params.length > 0)
      {
         // iterate through the parts of the message
         int count = 0;
         for(Iterator i = element.getChildren().iterator();
             i.hasNext(); count++)
         {
            XmlElement partElement = (XmlElement)i.next();
            
            // get the appropriate message part
            WsdlMessagePart part = message.getParts().getPart(
               partElement.getName());
            
            // get the parameter value
            String value = partElement.getValue();
            
            getLogger().debug(getClass(), "soap method parameter found.");
            getLogger().debugData(getClass(), 
               "soap method parameter found:" +
               "\nname=" + part.getName() +
               "\nvalue=" + value +
               "\ntype=" + part.getType());
            
            // parse the object
            params[count] = Wsdl.parseObject(value, part.getType());
         }
         
         getLogger().debug(getClass(), 
            "number of soap method parameters read: " + count);
      }
      
      // set the parameters
      setParameters(params);
   }
   
   /**
    * Converts a soap response from an XmlElement.
    * 
    * @param element the XmlElement to convert from.
    */
   protected void convertSoapResponseFromXmlElement(XmlElement element)   
   {
      // get the response message
      WsdlMessage message = getResponseMessage();
      
      // build a results array
      Object[] results = new Object[message.getParts().size()];
      
      if(results.length > 0)
      {
         // iterate through the parts of the message
         int count = 0;
         for(Iterator i = element.getChildren().iterator();
             i.hasNext(); count++)
         {
            XmlElement partElement = (XmlElement)i.next();
            
            // get the appropriate message part
            WsdlMessagePart part = message.getParts().getPart(
               partElement.getName());
            
            // get the parameter value
            String value = partElement.getValue();
            
            getLogger().debug(getClass(), "soap method result found.");
            getLogger().debugData(getClass(), 
               "soap method result found:" +
               "\nname=" + part.getName() +
               "\nvalue=" + value +
               "\ntype=" + part.getType());
         
            // parse the object
            results[count] = Wsdl.parseObject(value, part.getType());
         }
         
         getLogger().debug(getClass(), 
            "number of soap method results read: " + count);
      }
      
      // set the results
      setResults(results);
   }
   
   /**
    * Gets the current port type operation.
    * 
    * @return the current port type operation.
    */
   protected WsdlPortTypeOperation getPortTypeOperation()
   {
      return mPortTypeOperation;
   }
   
   /**
    * Gets the WSDL for this soap message.
    * 
    * @return the WSDL for this soap message.
    */
   public Wsdl getWsdl()
   {
      return mWsdl;
   }
   
   /**
    * Gets the port type for this soap message.
    * 
    * @return the port type for this soap message.
    */
   public WsdlPortType getPortType()
   {
      return mPortType;
   }
   
   /**
    * Sets the method for this soap message.
    * 
    * @param method the method.
    */
   public void setMethod(String method)
   {
      // try to find the operation for the given method
      findPortTypeOperation(method);

      // store method
      mMethod = method;
   }
   
   /**
    * Gets the method.
    * 
    * @return the method.
    */
   public String getMethod()
   {
      return mMethod;
   }
   
   /**
    * Sets the parameters for the method.
    * 
    * @param params the parameters for the method.
    */
   public void setParameters(Object[] params)
   {
      mParams = params;
      if(mParams == null)
      {
         mParams = new Object[0];
      }
   }
   
   /**
    * Gets the parameters for the method.
    * 
    * @return the parameters for the method.
    */
   public Object[] getParameters()
   {
      if(mParams == null)
      {
         mParams = new Object[0];
      }
      
      return mParams;
   }
   
   /**
    * Sets the results.
    * 
    * @param results the results.
    */
   public void setResults(Object[] results)
   {
      mResults = results;
      if(mResults == null)
      {
         mResults = new Object[0];
      }
   }
   
   /**
    * Gets the results.
    * 
    * @return the results.
    */
   public Object[] getResults()
   {
      if(mResults == null)
      {
         mResults = new Object[0];
      }
      
      return mResults;
   }
   
   /**
    * Sets the fault code. Whenever a fault code is set, the serializer
    * type will be set to SOAP_FAULT. The only acceptable strings are:
    * 
    * VersionMismatch.*
    * MustUnderstand.*
    * Client.*
    * Server.*
    * 
    * @param faultCode the fault code.
    */
   public void setFaultCode(String faultCode)
   {
      // FUTURE CODE: this code needs to actually check qualified names, etc
      // for now, it sets only the most generic types and cuts off sub types
      if(faultCode.startsWith("VersionMismatch"))
      {
         setXmlSerializerOptions(SOAP_FAULT);
         mFaultCode = FAULT_VERSION_MISMATCH;
      }
      else if(faultCode.startsWith("MustUnderstand"))
      {
         setXmlSerializerOptions(SOAP_FAULT);
         mFaultCode = FAULT_MUST_UNDERSTAND;
      }
      else if(faultCode.startsWith("Client"))
      {
         setXmlSerializerOptions(SOAP_FAULT);
         mFaultCode = FAULT_CLIENT;
      }
      else if(faultCode.startsWith("Server"))
      {
         setXmlSerializerOptions(SOAP_FAULT);
         mFaultCode = FAULT_SERVER;
      }
   }
   
   /**
    * Gets the fault code.
    * 
    * @return the fault code or null if none is set.
    */
   public String getFaultCode()
   {
      return mFaultCode;
   }
   
   /**
    * Sets the fault string. This is a human readable string that provides a
    * short explanation for the fault.
    * 
    * @param faultString the fault string.
    */
   public void setFaultString(String faultString)
   {
      mFaultString = faultString;
   }
   
   /**
    * Gets the fault string. This is a human readable string that provides a
    * short explanation for the fault.
    * 
    * @return the fault string.
    */
   public String getFaultString()
   {
      return mFaultString;
   }   
   
   /**
    * Sets the fault actor. The fault actor specifies who caused the
    * fault to happen within the soap message path. The fault actor is
    * a URI identifying the source of the fault (i.e. the destination of
    * the soap message). It need not always be present unless the application
    * raising the fault is not the destination.
    * 
    * @param faultActor the fault actor (the URI for the source of the fault).
    */
   public void setFaultActor(String faultActor)
   {
      mFaultActor = faultActor;
   }
   
   /**
    * Gets the fault actor. The fault actor specifies who caused the
    * fault to happen within the soap message path. The fault actor is
    * a URI identifying the source of the fault (i.e. the destination of
    * the soap message). It need not always be present unless the application
    * raising the fault is not the destination.
    * 
    * @return the fault actor (the URI for the source of the fault).
    */
   public String getFaultActor()
   {
      return mFaultActor;
   }   
   
   /**
    * Sets the client IP.
    * 
    * @param clientIP the client's IP.
    */
   public void setClientIP(String clientIP)
   {
      mClientIP = clientIP;
   }
   
   /**
    * Gets the client IP.
    * 
    * @return the client IP.
    */
   public String getClientIP()
   {
      return mClientIP;
   }
   
   /**
    * Sets the http header for this soap message.
    * 
    * @param header the http header for this soap message.
    */
   public void setHttpHeader(HttpHeader header)
   {
      mHttpHeader = header;
   }
   
   /**
    * Gets the http header for this soap message.
    * 
    * @return the http header for this soap message.
    */
   public HttpHeader getHttpHeader()
   {
      return mHttpHeader;
   }
   
   /**
    * Sets the http web connection to read or write soap attachments with.
    * 
    * @param hwc the http web connection to read or write soap attachments with.
    */
   public void setAttachmentWebConnection(HttpWebConnection hwc)
   {
      mAttachmentWebConnection = hwc;
   }
   
   /**
    * Gets the http web connection to read or write soap attachments with.
    * 
    * @return the http web connection to read or write soap attachments with.
    */
   public HttpWebConnection getAttachmentWebConnection()
   {
      return mAttachmentWebConnection;
   }
   
   /**
    * Sends an attachment for this soap message.
    * 
    * @param header the http body part header for the attachment.
    * @param is the input stream to read the attachment from.
    * @param lastAttachment true if this is the last attachment, false if not.
    * 
    * @return true if the attachment was written, false if not.
    */
   public boolean sendAttachment(
      HttpBodyPartHeader header, InputStream is, boolean lastAttachment)
   {
      boolean rval = false;
      
      // get attachment connection
      HttpWebConnection hwc = getAttachmentWebConnection();
      if(hwc != null)
      {
         // send the header
         if(hwc.sendHeader(header))
         {
            // send the body
            rval = hwc.sendBodyPartBody(is, getHttpHeader(),
               header, lastAttachment);
         }
      }
      
      return rval;
   }   
   
   /**
    * Receives an attachment for this soap message if there is one
    * to be received.
    * 
    * @param header the http body part header for the attachment.
    * @param os the output stream to write the attachment to.
    * 
    * @return true if an attachment was received, false if not. 
    */
   public boolean receiveAttachment(HttpBodyPartHeader header, OutputStream os)
   {
      boolean rval = false;
      
      // get attachment connection
      HttpWebConnection hwc = getAttachmentWebConnection();
      if(hwc != null && hasMoreAttachments())
      {
         // receive the header
         if(hwc.receiveHeader(header))
         {
            // receive the body
            rval = hwc.receiveBodyPartBody(os, getHttpHeader(), header);
         }
      }
      
      return rval;
   }
   
   /**
    * Returns true if there are more attachments to be received, false if not.
    * 
    * @return true if there are more attachments to be received, false if not.
    */
   public boolean hasMoreAttachments()
   {
      boolean rval = false;
      
      // get attachment connection
      HttpWebConnection hwc = getAttachmentWebConnection();
      if(hwc != null)
      {
         String endBoundary = getHttpHeader().getEndBoundary();
         
         // see if there is a last read boundary or if the last read
         // boundary is not the end boundary
         if(hwc.getLastReadBoundary() == null ||
            !hwc.getLastReadBoundary().equals(endBoundary))
         {
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Returns true if this soap message is a soap request.
    *
    * @return true if this soap message is a soap request, false if not.
    */
   public boolean isRequest()
   {
      return (getXmlSerializerOptions() == SOAP_REQUEST);
   }
   
   /**
    * Returns true if this soap message is a soap response.
    *
    * @return true if this soap message is a soap response, false if not.
    */
   public boolean isResponse()
   {
      return (getXmlSerializerOptions() == SOAP_RESPONSE);
   }
   
   /**
    * Returns true if this soap message is a soap fault.
    *
    * @return true if this soap message is a soap fault, false if not.
    */
   public boolean isFault()
   {
      return (getXmlSerializerOptions() == SOAP_FAULT);
   }
   
   /**
    * This method takes options that are used to configure
    * how to convert to and from xml.
    *
    * @param options the configuration options.
    * 
    * @return true if options successfully set, false if not.    
    */
   public boolean setXmlSerializerOptions(int options)
   {
      boolean rval = false;
      
      if(options == SOAP_REQUEST ||
         options == SOAP_RESPONSE ||
         options == SOAP_FAULT)
      {
         mXmlOptions = options;
         rval = true;
      }
      
      return rval;
   }

   /**
    * This method gets the options that are used to configure
    * how to convert to and from xml.
    *
    * @return the configuration options.
    */
   public int getXmlSerializerOptions()
   {
      return mXmlOptions;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      // this is according to the set schema (env)
      return "Envelope";
   }
   
   /**
    * Creates an XmlElement from this object.
    *
    * @return the XmlElement that represents this object.
    */
   public XmlElement convertToXmlElement()
   {
      // create xml element
      XmlElement element = new XmlElement(
         getRootTag(), "soap", Wsdl.WSDL_NAMESPACE_URI);
      
      // FUTURE CODE: the current implementation makes some assumptions about
      // soap encoding and so forth -- the same ones made by the Wsdl class
      element.addAttribute(
         "soapenv", ENVELOPE_NAMESPACE_URI, "xmlns", Wsdl.WSDL_NAMESPACE_URI);
      element.addAttribute(
         "xsd", XSD_NAMESPACE_URI, "xmlns", Wsdl.WSDL_NAMESPACE_URI);
      element.addAttribute(
         "xsi", XSI_NAMESPACE_URI, "xmlns", Wsdl.WSDL_NAMESPACE_URI);
      element.addAttribute(
         "enc", ENCODING_NAMESPACE_URI, "xmlns", Wsdl.WSDL_NAMESPACE_URI);
      element.addAttribute(
         "tns", getWsdl().getTargetNamespaceUri(),
         "xmlns", Wsdl.WSDL_NAMESPACE_URI);
      element.addAttribute(
         "encodingStyle", ENCODING_NAMESPACE_URI,
         "soapenv", ENVELOPE_NAMESPACE_URI);
      
      // create the envelope's body element
      XmlElement bodyElement = new XmlElement(
         "Body", "soapenv", ENVELOPE_NAMESPACE_URI);
      
      if(isRequest())
      {
         // get the request message
         WsdlMessage message = getRequestMessage();
         
         // create the operation element
         XmlElement operationElement = new XmlElement(
            getPortTypeOperation().getName(),
            "tns", getWsdl().getTargetNamespaceUri());
         
         // convert the parameters
         int count = 0;
         for(Iterator i = message.getParts().iterator(); i.hasNext(); count++)
         {
            WsdlMessagePart part = (WsdlMessagePart)i.next();
            
            // create part element
            XmlElement partElement = new XmlElement(part.getName());
            partElement.setValue("" + getParameters()[count]);
            
            // add part element to operation
            operationElement.addChild(partElement);
         }
         
         // add operation element to body
         bodyElement.addChild(operationElement);
      }
      else if(isResponse())
      {
         // get the response message
         WsdlMessage message = getResponseMessage();
         
         // create the message element
         XmlElement messageElement = new XmlElement(
            message.getName(), "tns", getWsdl().getTargetNamespaceUri());
         
         // convert the results
         int count = 0;
         for(Iterator i = message.getParts().iterator(); i.hasNext(); count++)
         {
            WsdlMessagePart part = (WsdlMessagePart)i.next();
            
            // create part element
            XmlElement partElement = new XmlElement(part.getName());
            partElement.setValue("" + getResults()[count]);
            
            // add part element to message
            messageElement.addChild(partElement);
         }
         
         // add message element to body
         bodyElement.addChild(messageElement);
      }
      else if(isFault())
      {
         // convert the fault
         XmlElement faultElement = new XmlElement(
            "Fault", "soapenv", ENVELOPE_NAMESPACE_URI);
         
         // faultcode
         XmlElement faultcodeElement = new XmlElement("faultcode");
         faultcodeElement.setValue("soapenv:" + getFaultCode());
         faultElement.addChild(faultcodeElement);
         
         // fault string
         XmlElement faultstringElement = new XmlElement("faultstring");
         faultstringElement.setValue(getFaultString());
         faultElement.addChild(faultstringElement);

         // fault actor
         XmlElement faultactorElement = new XmlElement("faultactor");
         faultactorElement.setValue(getFaultActor());
         faultElement.addChild(faultactorElement);
         
         // add fault element to body
         bodyElement.addChild(faultElement);
      }
      
      // add body element to main element
      element.addChild(bodyElement);
      
      // return element
      return element;      
   }
   
   /**
    * Converts this object from an XmlElement.
    *
    * @param element the XmlElement to convert from.
    * 
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXmlElement(XmlElement element)   
   {
      boolean rval = false;
      
      // FUTURE CODE: the current implementation makes some assumptions about
      // soap encoding and so forth -- the same ones made by the Wsdl class
      
      // get the envelope namespace prefix
      String envNs = element.findNamespace(ENVELOPE_NAMESPACE_URI);
      
      getLogger().detail(getClass(), "soap envelope namespace prefix=" + envNs);
      
      // get the body element of the envelope
      XmlElement bodyElement = element.getFirstChild("Body", envNs);
      if(bodyElement != null)
      {
         getLogger().detail(getClass(), "found soap envelope body...");
         
         // get the child in the body
         XmlElement child = bodyElement.getFirstChild();
         
         // determine if the child is a Fault or not
         if(child.getName().equals("Fault"))
         {
            // convert soap fault
            convertSoapFaultFromXmlElement(child);
            
            rval = true;
         }
         else
         {
            getLogger().debug(getClass(),
               "got soap envelope method/response," +
               "method/response=" + child.getName());
            
            if(isRequest())
            {
               // convert the request
               convertSoapRequestFromXmlElement(child);
            }
            else
            {
               // convert the response
               convertSoapResponseFromXmlElement(child);
            }
            
            rval = true;
         }
      }
      
      return rval;
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
