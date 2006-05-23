/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.StringReader;
import java.util.HashMap;
import java.util.Iterator;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

import com.db.common.ElementReader;
import com.db.common.IXMLSerializer;
import com.db.common.XMLCoder;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * This class represents a SOAP message.
 * 
 * @author Dave Longley
 */
public class SoapMessage implements IXMLSerializer
{
   /**
    * The xml schema for a soap message (the xsd).
    */
   public static final String XSD_NAMESPACE =
      "http://www.w3.org/2001/XMLSchema";
   
   /**
    * The xml schema instance for a soap message.
    */
   public static final String XSI_NAMESPACE =
      "http://www.w3.org/2001/XMLSchema-instance";

   /**
    * The soap namespace.
    */
   public static final String SOAP_NAMESPACE =
      "http://schemas.xmlsoap.org/wsdl/soap/";

   /**
    * The encoding schema.
    */
   public static final String ENCODING_SCHEMA =
      "http://schemas.xmlsoap.org/soap/encoding/";

   /**
    * The envelope schema.
    */
   public static final String ENVELOPE_SCHEMA =
      "http://schemas.xmlsoap.org/soap/envelope/";
   
   /**
    * The name of the method to execute.
    */
   protected String mMethod;
   
   /**
    * The parameters for the method to execute.
    */
   protected HashMap mParams;
   
   /**
    * The result of the executed method.
    */
   protected String mResult;
   
   /**
    * The xml namespace for this soap message. 
    */
   protected String mNamespace;
   
   /**
    * The address of the client for this soap message.
    */
   protected String mClientIP;
   
   /**
    * The fault code for this soap message, if any.
    */
   protected int mFaultCode;
   
   /**
    * The fault string for this soap message, if any.
    */
   protected String mFaultString;
   
   /**
    * The fault actor for this soap message, if any.
    */
   protected String mFaultActor;
   
   /**
    * An http web connection for reading or writing attachments.
    */
   protected HttpWebConnection mAttachmentWebConnection;
   
   /**
    * The boundary for attachments.
    */
   protected String mAttachmentBoundary;
   
   /**
    * A soap fault: when a version mismatch occurs.
    */
   protected final static int FAULT_VERSION_MISMATCH = 0;
   
   /**
    * A soap fault: when something must be understood but is not.
    */
   protected final static int FAULT_MUST_UNDERSTAND  = 1;
   
   /**
    * A soap fault: when a soap client faults.
    */
   protected final static int FAULT_CLIENT           = 2;
   
   /**
    * A soap fault: when a soap server faults.
    */
   protected final static int FAULT_SERVER           = 4;
   
   /**
    * XML serializer options.
    */
   protected int mXmlOptions;
   
   /**
    * An XML serializer option for converting to/from a soap request. 
    */
   protected final static int SOAP_REQUEST  = 0;
   
   /**
    * An XML serializer option for converting to/from a soap response.
    */
   protected final static int SOAP_RESPONSE = 1;
   
   /**
    * An XML serializer option for converting to/from a soap fault. 
    */
   protected final static int SOAP_FAULT    = 2;
   
   /**
    * Creates a new soap message.
    */
   public SoapMessage()
   {
      mParams = new HashMap();
      setSerializerOptions(SOAP_REQUEST);
      
      mClientIP = "0.0.0.0";
   }
   
   /**
    * Sets the namespace for this soap message.
    * 
    * @param namespace the namespace;
    */
   public void setNamespace(String namespace)
   {
      mNamespace = namespace;
   }

   /**
    * Gets the namespace for this soap message.
    * 
    * @return the namespace;
    */
   public String getNamespace()
   {
      return mNamespace;
   }
   
   /**
    * Sets the method.
    * 
    * @param method the method.
    */
   public void setMethod(String method)
   {
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
    * Sets the params.
    * 
    * @param params the params.
    */
   public void setParams(HashMap params)
   {
      mParams = params;
   }
   
   /**
    * Gets the params.
    * 
    * @return the params.
    */
   public HashMap getParams()
   {
      return mParams;
   }
   
   /**
    * Sets the result.
    * 
    * @param result the result.
    */
   public void setResult(String result)
   {
      mResult = result;
   }
   
   /**
    * Gets the result.
    * 
    * @return the result.
    */
   public String getResult()
   {
      return mResult;
   }
   
   /**
    * Sets the fault code. Whenever a fault code is set, the serializer
    * type will be set to SOAP_FAULT.
    * 
    * @param faultCode the fault code.
    */
   public void setFaultCode(int faultCode)
   {
      if(faultCode == FAULT_VERSION_MISMATCH ||
         faultCode == FAULT_MUST_UNDERSTAND ||
         faultCode == FAULT_CLIENT ||
         faultCode == FAULT_SERVER)
      {
         setSerializerOptions(SOAP_FAULT);
         mFaultCode = faultCode;
      }
   }
   
   /**
    * Sets the fault code string. Whenever a fault code is set, the serializer
    * type will be set to SOAP_FAULT. The only acceptable strings are:
    * 
    * VersionMismatch
    * MustUnderstand
    * Client
    * Server
    * 
    * @param faultCodeStr the fault code string.
    */
   public void setFaultCode(String faultCodeStr)
   {
      if(faultCodeStr.equalsIgnoreCase("VersionMismatch"))
      {
         setSerializerOptions(SOAP_FAULT);
         mFaultCode = FAULT_VERSION_MISMATCH;
      }
      else if(faultCodeStr.equalsIgnoreCase("MustUnderstand"))
      {
         setSerializerOptions(SOAP_FAULT);
         mFaultCode = FAULT_MUST_UNDERSTAND;
      }
      else if(faultCodeStr.equalsIgnoreCase("Client"))
      {
         setSerializerOptions(SOAP_FAULT);
         mFaultCode = FAULT_CLIENT;
      }
      else if(faultCodeStr.equalsIgnoreCase("Server"))
      {
         setSerializerOptions(SOAP_FAULT);
         mFaultCode = FAULT_SERVER;
      }
   }
   
   /**
    * Gets the fault code as a string.
    * 
    * @return the fault code as a string
    */
   public String getFaultCodeString()
   {
      String rval = "";
      
      switch(getFaultCode())
      {
         case FAULT_VERSION_MISMATCH:
            rval = "VersionMismatch";
            break;
         case FAULT_MUST_UNDERSTAND:
            rval = "MustUnderstand";
            break;
         case FAULT_CLIENT:
            rval = "Client";
            break;
         case FAULT_SERVER:
            rval = "Server";
            break;
      }
      
      return rval;
   }
   
   /**
    * Gets the fault code as an integer.
    * 
    * @return the fault code as an integer.
    */
   public int getFaultCode()
   {
      return mFaultCode;
   }
   
   /**
    * Sets the fault string.
    * 
    * @param faultString the fault string.
    */
   public void setFaultString(String faultString)
   {
      mFaultString = faultString;
   }
   
   /**
    * Gets the fault string.
    * 
    * @return the fault string.
    */
   public String getFaultString()
   {
      return mFaultString;
   }   
   
   /**
    * Sets the fault actor.
    * 
    * @param faultActor the fault actor.
    */
   public void setFaultActor(String faultActor)
   {
      mFaultActor = faultActor;
   }
   
   /**
    * Gets the fault actor.
    * 
    * @return the fault actor.
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
    * Sets the boundary for soap attachments.
    *
    * @param boundary the boundary for soap attachments.
    */
   public void setAttachmentBoundary(String boundary)
   {
      mAttachmentBoundary = boundary;
   }
   
   /**
    * Gets the boundary for soap attachments.
    * 
    * @return the boundary for soap attachments.
    */
   public String getAttachmentBoundary()
   {
      return mAttachmentBoundary;
   }
   
   /**
    * Sends an attachment for this soap message.
    * 
    * @param header the http body part header for the attachment.
    * @param is the input stream to read the attachment from.
    * @param lastAttachment true if this is the last attachment, false if not.
    * @return true if the attachment was written, false if not.
    */
   public boolean sendAttachment(HttpBodyPartHeader header, InputStream is,
                                 boolean lastAttachment)
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
            rval = hwc.sendBodyPartBody(is, header.getContentEncoding(),
                                        getAttachmentBoundary(),
                                        lastAttachment);
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
            rval = hwc.receiveBodyPartBody(os, header.getContentEncoding(),
                                           getAttachmentBoundary());
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
         String endBoundary = getAttachmentBoundary() + "--";
         
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
    * This method takes options that are used to configure
    * how to convert to and from xml.
    *
    * @param options the configuration options.
    * @return true if options successfully set, false if not.    
    */
   public boolean setSerializerOptions(int options)
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
   public int getSerializerOptions()
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
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    * 
    * @return the xml-based representation of the object.
    */
   public String convertToXML()
   {
      return convertToXML(0);
   }
   
   /**
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    *
    * @param indentLevel the number of spaces to place before the text
    *                    after each new line.
    * @return the xml-based representation of the object.
    */
   public String convertToXML(int indentLevel)
   {
      // TEMP CODE: this code makes a couple of assumptions
      // like "result" will indicate a method result
      
      StringBuffer xml = new StringBuffer();
      StringBuffer indent = new StringBuffer("\n");
      for(int i = 0; i < indentLevel; i++)
      {
         indent.append(' ');
      }

      if(indentLevel == 0)
      {
         xml.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
      }
      
      xml.append("<soap:Envelope xmlns:soap=\"" + ENVELOPE_SCHEMA + "\" ");
      xml.append("xmlns:xsd=\"" + XSD_NAMESPACE + "\" ");
      xml.append("xmlns:xsi=\"" + XSI_NAMESPACE + "\" ");
      xml.append("xmlns:enc=\"" + ENCODING_SCHEMA + "\" ");
      xml.append("xmlns:tns=\"" + getNamespace() + "\" ");
      xml.append("soap:encodingStyle=\"" + ENCODING_SCHEMA +"\">");
      
      // add the envelope's body
      xml.append("<soap:Body>");

      if(getSerializerOptions() == SOAP_REQUEST)
      {
         xml.append("<tns:" + getMethod() + ">");
         
         Iterator i = getParams().keySet().iterator();
         while(i.hasNext())
         {
            String paramName = (String)i.next();
            String paramValue = "" + getParams().get(paramName);
            xml.append("<" + paramName + ">");
            xml.append(XMLCoder.encode(paramValue));
            xml.append("</" + paramName + ">");
         }
         
         xml.append("</tns:" + getMethod() + ">");
      }
      else if(getSerializerOptions() == SOAP_RESPONSE)
      {
         xml.append("<tns:" + getMethod() + "Response>");
         xml.append("<result>" + XMLCoder.encode(getResult()) + "</result>");
         xml.append("</tns:" + getMethod() + "Response>");
      }
      else if(getSerializerOptions() == SOAP_FAULT)
      {
         xml.append("<soap:Fault>");
         xml.append("<faultcode>soap:" + XMLCoder.encode(getFaultCodeString()) +
                    "</faultcode>");
         xml.append("<faultstring>" + XMLCoder.encode(getFaultString()) +
                    "</faultstring>");
         xml.append("<faultactor>" + XMLCoder.encode(getFaultActor()) +
                    "</faultactor>");
         xml.append("</soap:Fault>");
      }
      
      xml.append("</soap:Body>");
      xml.append("</soap:Envelope>");

      return xml.toString();
   }

   /**
    * This method takes XML text (in full document form) and converts
    * it to it's internal representation.
    *
    * @param xmlText the xml text document that represents the object.
    * @return true if successful, false otherwise.    
    */
   public boolean convertFromXML(String xmlText)
   {
      boolean rval = false;
      
      getLogger().debug("Loading soap message from xml:\n" + xmlText);

      try
      {
         DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
         DocumentBuilder builder = factory.newDocumentBuilder();
         
         InputSource is = new InputSource(new StringReader(xmlText));
         Document doc = builder.parse(is);
         
         // normalize text representation
         doc.getDocumentElement().normalize();
         
         rval = convertFromXML(doc.getDocumentElement());
      }
      catch(SAXParseException spe)
      {
         getLogger().debug("SoapMessage parsing error"
                            + ", line " + spe.getLineNumber()
                            + ", uri " + spe.getSystemId());
         getLogger().debug("   " + spe.getMessage());
         // print stack trace as below
      }
      catch(SAXException se)
      {
         Exception x = se.getException();
         x = (x == null) ? se : x;
         
         getLogger().debug(Logger.getStackTrace(x));
      }
      catch(Exception e)
      {
         getLogger().debug(Logger.getStackTrace(e));
      }      
      
      return rval;
   }
   
   /**
    * This method takes a parsed DOM XML element and converts it
    * back into this object's representation.
    *
    * @param element the parsed element that contains this objects information.
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXML(Element element)
   {
      boolean rval = false;
      
      // TEMP CODE: this code makes a couple of assumptions
      // like "result" will indicate a method result
      
      // reset method and params
      setMethod("");
      setParams(new HashMap());
      
      String namespace = "";

      ElementReader er = new ElementReader(element);
      
      // iterate through the envelope elements
      Iterator ei = er.getElementReadersNS(ENVELOPE_SCHEMA).iterator();
      while(ei.hasNext())
      {
         getLogger().debug("found envelope elements...");
         
         // get the envelope prefix
         String envPrefix = er.getPrefix(ENVELOPE_SCHEMA, false);
         getLogger().debug("envelope prefix=" + envPrefix);
         
         ElementReader envelopeER = (ElementReader)ei.next();
         if(envelopeER.getTagName().equals(envPrefix + ":Body"))
         {
            getLogger().debug("got envelope body...");
            
            // go through all of the body (method/response) elements
            Iterator bi = envelopeER.getElementReaders().iterator();
            while(bi.hasNext() && !rval)
            {
               ElementReader bodyER = (ElementReader)bi.next();
               String name[] = bodyER.getTagName().split(":"); 
               if(name.length > 1)
               {
                  // see if this is a soap fault
                  if(name[1].equals("Fault"))
                  {
                     // get fault code, fault string, and fault actor
                     ElementReader r =
                        bodyER.getFirstElementReader("faultcode");
                     if(r != null)
                     {
                        String[] split = r.getStringValue().split(":");
                        if(split.length > 1)
                        {
                           setFaultCode(split[1]);
                        }
                     }
                     
                     r = bodyER.getFirstElementReader("faultstring");
                     if(r != null)
                     {
                        setFaultString(r.getStringValue());
                     }

                     r = bodyER.getFirstElementReader("faultactor");
                     if(r != null)
                     {
                        setFaultActor(r.getStringValue());
                     }
                     
                     rval = true;
                  }
                  else
                  {
                     getLogger().debug("got envelope method/response," +
                                       "method/response=" + name[1]);
                     
                     // if namespace not set, look it up
                     if(namespace.equals(""))
                     {
                        namespace = bodyER.getNamespaceURI(name[0], false);
                        setNamespace(namespace);
                     }
                     
                     // set the method
                     setMethod(name[1]);
                     
                     // set parameters
                     Iterator pi = bodyER.getElementReaders().iterator();
                     while(pi.hasNext())
                     {
                        ElementReader paramER = (ElementReader)pi.next();
                        String paramName = paramER.getTagName();
                        String paramValue = paramER.getStringValue();

                        // assumes "result" is being used
                        if(paramName.equals("result"))
                        {
                           getLogger().debug("result found,result=" +
                                             paramValue);
                           setResult(paramValue);
                        }
                        else
                        {
                           getLogger().debug("param found,name=" + paramName +
                                             ",value=" + paramValue);
                           
                           // add the param to the map
                           mParams.put(paramName, paramValue);
                        }
                     }

                     getLogger().debug("number of parameters read: " +
                                       mParams.size());
                     
                     rval = true;
                  }
               }
            }
            
            break;
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
      return LoggerManager.getLogger("dbcommon");
   }
}
