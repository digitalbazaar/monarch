/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import java.util.Iterator;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.IXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A RpcSoapEnvelope is a SoapEnvelope that holds either a SoapOperation
 * or a SoapFault. It is specifically used to perform remote procedure calls
 * (via SoapOperations) over SOAP (Simple Object Access Protocol).
 * 
 * @author Dave Longley
 */
public class RpcSoapEnvelope extends SoapEnvelope
{
   /**
    * The SoapOperation this envelope contains, if any.
    */
   protected SoapOperation mSoapOperation;
   
   /**
    * The SoapFault this envelope contains, if any.
    */
   protected SoapFault mSoapFault;
   
   /**
    * Creates a new empty RpcSoapEnvelope.
    */
   public RpcSoapEnvelope()
   {
      // no operation or fault by default
      mSoapOperation = null;
      mSoapFault = null;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      // this is according to the soap envelope schema
      return "Envelope";
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
      
      // clear soap operation and soap fault
      mSoapOperation = null;
      mSoapFault = null;
      
      // create a new soap operation and soap fault
      SoapOperation operation = new SoapOperation();
      SoapFault fault = new SoapFault();
      
      // convert the base class soap envelope
      if(super.convertFromXmlElement(element))
      {
         // look for a soap fault or soap operation in the body content of
         // the envelope
         boolean foundFault = false;
         boolean foundOperation = false;
         for(Iterator<IXmlSerializer> i = getBodyContents().iterator();
             i.hasNext() && !foundOperation && !foundFault;)
         {
            XmlElement contentElement = (XmlElement)i.next();
            
            // try to convert a fault
            if(contentElement.getRootTag().equals(fault.getRootTag()))
            {
               foundFault = fault.convertFromXmlElement(contentElement);
            }
            else
            {
               // try to convert an operation
               foundOperation = operation.convertFromXmlElement(contentElement);
            }
         }
         
         // determine if a fault or an operation was found
         if(foundFault)
         {
            // set soap fault
            setSoapFault(fault);
            rval = true;
         }
         else if(foundOperation)
         {
            // set soap operation
            setSoapOperation(operation);
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Returns true if this envelope contains a SoapOperation, false if
    * it does not.
    * 
    * @return true if this envelope contains a SoapOperation, false if
    *         it does not.
    */
   public boolean containsSoapOperation()
   {
      boolean rval = false;
      
      if(mSoapOperation != null)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Sets the SoapOperation for this envelope. Any SoapFault will be removed.
    * 
    * @param operation the SoapOperation for this envelope.
    */
   public void setSoapOperation(SoapOperation operation)
   {
      mSoapOperation = operation;
      mSoapFault = null;
      
      // clear body content from envelope
      clearBodyContents();
      
      // add body content serializer
      addBodyContentSerializer(operation);
   }
   
   /**
    * If this envelope contains a SoapOperation, then this method will return
    * it. If it does not, then this method will return null. 
    * 
    * @return the SoapOperation contained in this envelope, or null.
    */
   public SoapOperation getSoapOperation()
   {
      SoapOperation rval = null;
      
      if(containsSoapOperation())
      {
         rval = mSoapOperation;
      }
      
      return rval;      
   }
   
   /**
    * Returns true if this envelope contains a SoapFault, false if
    * it does not.
    * 
    * @return true if this envelope contains a SoapFault, false if
    *         it does not.
    */
   public boolean containsSoapFault()
   {
      boolean rval = false;
      
      if(mSoapFault != null)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Sets the SoapFault for this envelope. Any SoapOperation in this envelope
    * will be removed.
    * 
    * @param fault the SoapFault to set.
    */
   public void setSoapFault(SoapFault fault)
   {
      mSoapFault = fault;
      mSoapOperation = null;
      
      // clear body content from envelope
      clearBodyContents();
      
      // add body content serializer
      addBodyContentSerializer(fault);
   }
   
   /**
    * If this envelope contains a SoapFault, then this method will return
    * it. If it does not, then this method will return null.
    * 
    * @return the SoapFault contained in this envelope, or null.
    */
   public SoapFault getSoapFault()
   {
      SoapFault rval = null;
      
      if(containsSoapFault())
      {
         rval = mSoapFault;
      }
      
      return rval;
   }
   
   /**
    * Gets the logger for this RpcSoapEnvelope.
    * 
    * @return the logger for this RpcSoapEnvelope.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }   
}
