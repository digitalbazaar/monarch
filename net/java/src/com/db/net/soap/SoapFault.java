/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A SoapFault is a class that represents a SOAP (Simple Object Access Protocol)
 * Fault message. A SOAP fault is "raised" when some error occurs during
 * communication over SOAP.
 * 
 * FUTURE CODE: This soap fault implementation may be incomplete/incorrect
 * with respect to fault code and fault detail.
 * 
 * @author Dave Longley
 */
public class SoapFault extends AbstractXmlSerializer
{
   /**
    * The fault code for this soap fault, if any. A fault code can be any
    * of the following:
    * 
    * VersionMismatch.*
    * MustUnderstand.*
    * Client.*
    * Server.*
    */
   protected String mFaultCode;
   
   /**
    * The fault string for this soap fault, if any. This is a human
    * readable string that provides a short explanation for the fault.
    */
   protected String mFaultString;
   
   /**
    * The fault actor for this soap fault, if any. The fault actor specifies
    * who caused the fault to happen within the soap message path. The fault
    * actor is a URI identifying the source of the fault.
    */
   protected String mFaultActor;
   
   /**
    * The fault detail for this soap fault, if any. This is an XmlElement
    * defining, in some specific way, a detailed error message. Parsing
    * this element is left up to the user of an instance of SoapFault.
    */
   protected XmlElement mFaultDetail;
   
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
    * Creates a new SoapFault.
    */
   public SoapFault()
   {
      // set defaults
      setFaultCode("VersionMismatch");
      setFaultString("");
      setFaultActor("");
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      // according to soap envelope schema
      return "Fault";
   }
   
   /**
    * Creates an XmlElement from this object.
    *
    * @param parent the parent XmlElement for the XmlElement being created
    *               (can be null). 
    * 
    * @return the XmlElement that represents this object.
    */
   public XmlElement convertToXmlElement(XmlElement parent)   
   {
      // create the root element
      XmlElement faultElement =
         new XmlElement(getRootTag(), SoapEnvelope.SOAP_ENVELOPE_URI);
      faultElement.setParent(parent);
      
      // get the soap envelope namespace prefix
      String envelopePrefix = faultElement.findNamespacePrefix(
         SoapEnvelope.SOAP_ENVELOPE_URI);
      
      // create the faultcode element
      XmlElement faultcodeElement = new XmlElement("faultcode");
      faultcodeElement.setInheritNamespaceUri(false);
      faultcodeElement.setValue(envelopePrefix + ":" + getFaultCode());
      faultElement.addChild(faultcodeElement);
      
      // create the faultstring element
      XmlElement faultstringElement = new XmlElement("faultstring");
      faultstringElement.setInheritNamespaceUri(false);
      faultstringElement.setValue(getFaultString());
      faultElement.addChild(faultstringElement);
     
      // create the fault actor element
      XmlElement faultactorElement = new XmlElement("faultactor");
      faultactorElement.setInheritNamespaceUri(false);
      faultactorElement.setValue(getFaultActor());
      faultElement.addChild(faultactorElement);

      if(getFaultDetail() != null)
      {
         // create the faultdetail element, if any
         XmlElement faultdetailElement = new XmlElement("detail");
         faultdetailElement.setInheritNamespaceUri(false);
         faultdetailElement.addChild(getFaultDetail());
         faultElement.addChild(faultdetailElement);
      }
      
      // return the fault element
      return faultElement;
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
      boolean rval = true;
      
      // get the faultcode element
      XmlElement faultcodeElement = element.getFirstChild("faultcode");
      if(faultcodeElement != null)
      {
         // remove any namespace prefix for the fault code
         setFaultCode(XmlElement.parseLocalName(faultcodeElement.getValue()));
      }
      
      // get the faultstring element
      XmlElement faultstringElement = element.getFirstChild("faultcode");
      if(faultstringElement != null)
      {
         setFaultString(faultstringElement.getValue());
      }
      
      // get the faultactor element
      XmlElement faultactorElement = element.getFirstChild("faultcode");
      if(faultactorElement != null)
      {
         setFaultActor(faultactorElement.getValue());
      }
      
      // get the faultdetail element, if any
      XmlElement faultdetailElement = element.getFirstChild("detail");
      if(faultdetailElement != null)
      {
         setFaultDetail(faultdetailElement.getFirstChild());
      }
      else
      {
         // no fault detail element
         mFaultDetail = null;
      }
      
      return rval;
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
      if(faultCode.startsWith("VersionMismatch"))
      {
         mFaultCode = FAULT_VERSION_MISMATCH;
      }
      else if(faultCode.startsWith("MustUnderstand"))
      {
         mFaultCode = FAULT_MUST_UNDERSTAND;
      }
      else if(faultCode.startsWith("Client"))
      {
         mFaultCode = FAULT_CLIENT;
      }
      else if(faultCode.startsWith("Server"))
      {
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
    * Sets the fault detail. The fault detail is an XmlElement that has
    * a detailed message encoded in XML. Parsing the fault detail element
    * is left up to the user of a SoapFault instance.
    * 
    * @param faultDetail the fault detail (an XmlElement).
    */
   public void setFaultDetail(XmlElement faultDetail)
   {
      mFaultDetail = faultDetail;
   }
   
   /**
    * Gets the fault detail. The fault detail is an XmlElement that has
    * a detailed message encoded in XML. Parsing the fault detail element
    * is left up to the user of a SoapFault instance.
    * 
    * @return the fault detail (an XmlElement that may be null).
    */
   public XmlElement getFaultDetail()
   {
      return mFaultDetail;
   }
   
   /**
    * Gets the logger for this SoapOperation.
    * 
    * @return the logger for this SoapOperation.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }   
}
