/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A UPnPError is a error that occurred while communicating using UPnP. 
 * 
 * @author Dave Longley
 */
public class UPnPError extends AbstractXmlSerializer
{
   /**
    * The error code for this error.
    */
   protected int mErrorCode;
   
   /**
    * The error description for this error.
    */
   protected String mErrorDescription;
   
   /**
    * The XML namespace URI for a UPnPError.
    */
   public static final String XML_NAMESPACE_URI =
      "urn:schemas-upnp-org:control-1-0";
   
   /**
    * Creates a new UPnPError.
    */
   public UPnPError()
   {
      // set defaults
      setErrorCode(0);
      setErrorDescription("");
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "UPnPError";
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
      XmlElement errorElement = new XmlElement(getRootTag(), XML_NAMESPACE_URI);
      errorElement.setParent(parent);
      
      // add the attribute defining the namespace
      errorElement.addAttribute("xmlns", XML_NAMESPACE_URI);
      
      // create error code element
      XmlElement errorCodeElement = new XmlElement("errorCode");
      errorCodeElement.setValue(getErrorCode());
      errorElement.addChild(errorCodeElement);
      
      // create error description element
      XmlElement errorDescriptionElement = new XmlElement("errorDescription");
      errorDescriptionElement.setValue(getErrorDescription());
      errorElement.addChild(errorDescriptionElement);
      
      // return root element
      return errorElement;
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
      
      // check the namespace
      if(element.getNamespaceUri().equals(XML_NAMESPACE_URI))
      {
         rval = true;
         
         // get error code element
         setErrorCode(Integer.parseInt(
            element.getFirstChildValue("errorCode")));
         
         // get error description element
         setErrorDescription(element.getFirstChildValue("errorDescription"));
      }
      
      return rval;
   }
   
   /**
    * Sets the error code for this error.
    * 
    * @param code the error code for this error.
    */
   public void setErrorCode(int code)
   {
      mErrorCode = code;
   }
   
   /**
    * Gets the error code for this error.
    * 
    * @return the error code for this error.
    */
   public int getErrorCode()
   {
      return mErrorCode;
   }   
   
   /**
    * Sets the error description for this error.
    * 
    * @param description the error description for this error.
    */
   public void setErrorDescription(String description)   
   {
      mErrorDescription = description;
   }
   
   /**
    * Gets the error description for this error.
    * 
    * @return the error description for this error.
    */
   public String getErrorDescription()   
   {
      return mErrorDescription;
   }
   
   /**
    * Gets the logger for this UPnPError.
    * 
    * @return the logger for this UPnPError.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
