/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.wsdl.Wsdl;
import com.db.net.wsdl.WsdlBinding;
import com.db.net.wsdl.WsdlPort;
import com.db.xml.XmlElement;

/**
 * A WSDL Soap Port.
 * 
 * A WSDL Soap Port describes a location that provides operations for a
 * web service using SOAP over HTTP.
 * 
 * @author Dave Longley
 */
public class WsdlSoapPort extends WsdlPort
{
   /**
    * The URI for this port.
    */
   protected String mUri; 
   
   /**
    * Creates a new blank WsdlSoapPort.
    * 
    * @param binding the binding for this port.
    */
   public WsdlSoapPort(WsdlBinding binding)
   {
      this(binding, "", "");
   }
   
   /**
    * Creates a new WsdlSoapPort with the given name.
    * 
    * @param binding the binding for this port.
    * @param name the name of this port.
    */
   public WsdlSoapPort(WsdlBinding binding, String name)
   {
      this(binding, name, "");
   }   
   
   /**
    * Creates a new WsdlSoapPort with the given name.
    * 
    * @param binding the binding for this port.
    * @param name the name of this port.
    * @param uri the URI for this port.
    */
   public WsdlSoapPort(WsdlBinding binding, String name, String uri)
   {
      super(binding, name);
      
      // set uri
      setUri(uri);
   }
   
   /**
    * Sets the URI for this port.
    * 
    * @param uri the uri for this port.
    */
   public void setUri(String uri)
   {
      mUri = uri;
   }
   
   /**
    * Gets the URI for this port.
    * 
    * @return the uri for this port.
    */
   public String getUri()
   {
      return mUri;
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
      // create the base port xml element
      XmlElement element = createPortXmlElement(parent);
      
      // add the soap address element
      XmlElement soapAddressElement = new XmlElement(
         "address", "soap", Wsdl.WSDL_SOAP_NAMESPACE_URI);
      soapAddressElement.addAttribute("location", getUri());
      soapAddressElement.addAttribute("xmlns", Wsdl.WSDL_NAMESPACE_URI);
      element.addChild(soapAddressElement);
      
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
      
      // clear name
      setName("");
      
      // clear uri
      setUri("");
      
      if(element.getName().equals(getRootTag()))
      {
         // get name
         setName(element.getAttributeValue("name"));
         
         // get soap address element
         XmlElement soapAddressElement = element.getFirstChild(
            "address", Wsdl.WSDL_SOAP_NAMESPACE_URI);
         if(soapAddressElement != null)
         {
            setUri(soapAddressElement.getAttributeValue("location"));
            
            // ensure there is a name
            if(!getName().equals(""))            
            {
               // conversion successful
               rval = true;
            }
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
