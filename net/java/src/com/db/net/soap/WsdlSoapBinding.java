/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.wsdl.Wsdl;
import com.db.net.wsdl.WsdlBinding;
import com.db.net.wsdl.WsdlPortType;
import com.db.net.wsdl.WsdlPortTypeOperation;
import com.db.xml.XmlElement;

/**
 * A WSDL SOAP Binding.
 * 
 * A WSDL Binding binds a port type's operations to a particular message
 * format and transmission protocol. This implementation binds a port type
 * to a SOAP encoding and SOAP over HTTP transmission protocol.
 * 
 * FUTURE CODE:
 * The current implementation assumes "rpc" style transportation will be
 * used not "literal." This can be updated in the future.
 *  
 * @author Dave Longley
 */
public class WsdlSoapBinding extends WsdlBinding
{
   /**
    * The operations for this binding.
    */
   protected WsdlSoapBindingOperationCollection mOperationCollection;
   
   /**
    * The SOAP HTTP namespace.
    */
   public static final String SOAP_OVER_HTTP_NAMESPACE_URI =
      "http://schemas.xmlsoap.org/soap/http";
   
   /**
    * Creates a new blank WsdlSoapBinding.
    * 
    * @param wsdl the wsdl this binding is associated with.
    */
   public WsdlSoapBinding(Wsdl wsdl)   
   {
      this(wsdl, "", null);
   }

   /**
    * Creates a new WsdlSoapBinding with the given name and port type.
    * 
    * @param wsdl the wsdl this binding is associated with.
    * @param name the name of this binding.
    * @param portType the port type for this binding.
    */
   public WsdlSoapBinding(Wsdl wsdl, String name, WsdlPortType portType)
   {
      super(wsdl, name, portType);
   }
   
   /**
    * Overridden to create a soap binding operation for each port type
    * operation. 
    * 
    * Sets the port type for this binding.
    * 
    * @param portType the port type for this binding.
    */
   public void setPortType(WsdlPortType portType)
   {
      super.setPortType(portType);
      
      // clear old soap binding operations
      getOperations().clear();
      
      if(portType != null)
      {
         // create corresponding soap binding operations
         for(Iterator i = portType.getOperations().iterator(); i.hasNext();)
         {
            WsdlPortTypeOperation portTypeOperation =
               (WsdlPortTypeOperation)i.next();

            // create soap binding operation and add it
            WsdlSoapBindingOperation operation =
               new WsdlSoapBindingOperation(portTypeOperation);
            getOperations().add(operation);
         }
      }
   }
   
   /**
    * Gets the operations for this binding.
    * 
    * @return the operations for this binding.
    */
   public WsdlSoapBindingOperationCollection getOperations()
   {
      if(mOperationCollection == null)
      {
         // create operations collection
         mOperationCollection = new WsdlSoapBindingOperationCollection();
      }
      
      return mOperationCollection;
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
      // create xml element
      XmlElement element = new XmlElement(getRootTag());
      element.setParent(parent);

      // add attributes
      element.addAttribute("name", getName());
      element.addAttribute("type", "tns:" + getPortType().getName());

      // add soap transport element
      XmlElement soapTransportElement = new XmlElement(
         "binding", "soap", Wsdl.WSDL_SOAP_NAMESPACE_URI);
      soapTransportElement.addAttribute(
         "transport", SOAP_OVER_HTTP_NAMESPACE_URI);
      soapTransportElement.addAttribute("style", "rpc");
      element.addChild(soapTransportElement);
      
      // add soap binding operations
      for(Iterator i = getOperations().iterator(); i.hasNext();)
      {
         WsdlSoapBindingOperation operation =
            (WsdlSoapBindingOperation)i.next();
         element.addChild(operation.convertToXmlElement(element));
      }
      
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
      
      // clear port type
      setPortType(null);
      
      // clear operations
      getOperations().clear();

      if(element.getName().equals(getRootTag()))
      {
         // get name
         String name = element.getAttributeValue("name");
         
         // FIXME: strip the namespace prefix
         name = XmlElement.getBasicName(name);

         // set name
         setName(name);
         
         // get the port type
         String type = element.getAttributeValue("type");
         setPortType(getWsdl().getPortTypes().getPortType(type));
         
         // get the operations
         if(getPortType() != null)
         {
            boolean error = false;
            for(Iterator i = element.getChildren("operation").iterator();
                i.hasNext() && !error;)
            {
               XmlElement operationElement = (XmlElement)i.next();
               
               // get the operation name
               String operationName = 
                  operationElement.getAttributeValue("name");
               
               // FIXME: strip the namespace prefix
               operationName = XmlElement.getBasicName(operationName);
               
               // get the WsdlSoapBindingOperation and convert it
               WsdlSoapBindingOperation operation =
                  getOperations().getOperation(operationName);
               if(operation != null)
               {
                  if(operation.convertFromXmlElement(operationElement))
                  {
                     // operation converted, add it
                     getOperations().add(operation);
                  }
                  else
                  {
                     error = true;
                  }
               }
               else
               {
                  error = true;
                  getLogger().error(getClass(),
                     "No valid Wsdl Soap Binding Operation found!" +
                     ",operation_name=" + operationName);
               }
            }
            
            // ensure there is a name an no errors
            if(!getName().equals("") && !error)            
            {
               // conversion successful
               rval = true;
            }            
         }
         else
         {
            getLogger().error(getClass(),
               "No valid Wsdl Port Type found!,type=" + type);
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
   
   /**
    * A WSDL SOAP Binding Operation collection.
    * 
    * @author Dave Longley
    */
   public class WsdlSoapBindingOperationCollection
   {
      /**
       * The underlying vector for storing operations. 
       */
      protected Vector mOperations;
      
      /**
       * Creates a new Wsdl SOAP Binding Operation Collection.
       */
      public WsdlSoapBindingOperationCollection()
      {
         // initialize operations vector
         mOperations = new Vector();
      }
      
      /**
       * Adds a new operation to this collection.
       * 
       * @param operation the operation to add to this collection.
       */
      public void add(WsdlSoapBindingOperation operation)
      {
         mOperations.add(operation);
      }
      
      /**
       * Removes a operation from this collection.
       * 
       * @param operation the operation to remove from this collection.
       */
      public void remove(WsdlSoapBindingOperation operation)
      {
         mOperations.remove(operation);
      }
      
      /**
       * Gets an operation from this collection according to its name.
       * 
       * @param name the name of the operation to retrieve.
       * 
       * @return the operation or null if one was not found.
       */
      public WsdlSoapBindingOperation getOperation(String name)
      {
         WsdlSoapBindingOperation rval = null;
         
         for(Iterator i = iterator(); i.hasNext() && rval == null;) 
         {
            WsdlSoapBindingOperation operation =
               (WsdlSoapBindingOperation)i.next();
            if(operation.getName().equals(name))
            {
               rval = operation;
            }
         }
         
         return rval;
      }      
      
      /**
       * Clears all the operation from this collection.
       */
      public void clear()
      {
         mOperations.clear();
      }
      
      /**
       * Gets an iterator over the operation in this collection.
       * 
       * @return an iterator over the operation in this collection.
       */
      public Iterator iterator()
      {
         return mOperations.iterator();
      }
      
      /**
       * Gets the number of operations in this collection.
       * 
       * @return the number of operations in this collection.
       */
      public int size()
      {
         return mOperations.size();
      }
   }   
}
