/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import java.util.Iterator;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.soap.AbstractSoapWebService.SoapMethodNotRecognizedException;
import com.db.net.wsdl.Wsdl;
import com.db.net.wsdl.WsdlMessage;
import com.db.net.wsdl.WsdlMessagePart;
import com.db.net.wsdl.WsdlPortTypeOperation;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.IXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A WSDL SOAP Binding Operation.
 * 
 * A WSDL SOAP Binding Operation defines the message encoding and 
 * transmission protocol for a WSDL Port Type operation. It indicates
 * that messages will be encoded in a SOAP envelope and sent over HTTP.
 * 
 * FUTURE CODE: When we move over to SOAP 1.2 or (even before that if we
 * add complex object support) we want a clean redesign of the soap classes
 * and interfaces to make it more streamlined and easy to use. This may
 * include created a new interface for serializing complex objects to xml.  
 * 
 * @author Dave Longley
 */
public class WsdlSoapBindingOperation extends AbstractXmlSerializer
{
   /**
    * The WsdlSoapBinding this operation is for.
    */
   protected WsdlSoapBinding mBinding;
   
   /**
    * The corresponding port type operation.
    */
   protected WsdlPortTypeOperation mPortTypeOperation;
   
   /**
    * The optional soap action associated with this operation. 
    */
   protected String mSoapAction;
   
   /**
    * The SOAP encoding namespace.
    */
   public static final String SOAP_ENCODING_NAMESPACE_URI =
      "http://schemas.xmlsoap.org/soap/encoding/";   
   
   /**
    * Creates a new Wsdl Soap Binding Operation with the given
    * associated port type operation.
    * 
    * @param binding the WsdlSoapBinding this operation is for.
    * @param operation the associated port type operation.
    */
   public WsdlSoapBindingOperation(
      WsdlSoapBinding binding, WsdlPortTypeOperation operation)
   {
      // store binding
      mBinding = binding;
      
      // store port type operation
      setPortTypeOperation(operation);
      
      // set blank soap action
      setSoapAction("");
   }
   
   /**
    * Adds the SOAP body element child to the passed XmlElement.
    * 
    * @param element the XmlElement to add the SOAP body element to.
    */
   protected void addSoapBodyToXmlElement(XmlElement element)
   {
      // create the soap body element
      XmlElement soapBodyElement = new XmlElement(
         "body", "soap", Wsdl.WSDL_SOAP_NAMESPACE_URI);
      soapBodyElement.addAttribute(
         "encodingStyle", SOAP_ENCODING_NAMESPACE_URI);
      soapBodyElement.addAttribute("use", "encoded");
      
      // add the soap body element as a child
      element.addChild(soapBodyElement);
   }
   
   /**
    * Creates a SoapOperation from a WsdlMessage and a set of parameter values.
    * 
    * @param message the WsdlMessage to create the SoapOperation from.
    * @param params the parameter values.
    * 
    * @return the SoapOperation.
    */
   protected SoapOperation createSoapOperation(
      WsdlMessage message, Object[] params)
   {
      // create soap operation
      SoapOperation operation = new SoapOperation(
         message.getName(), message.getNamespaceUri());
      
      // add soap operation parameters according to wsdl message
      int count = 0;
      for(Iterator i = message.getParts().iterator(); i.hasNext(); count++)
      {
         WsdlMessagePart part = (WsdlMessagePart)i.next();
         
         // create a soap operation parameter
         SoapOperationParameter parameter = null;
         if(params[count] instanceof IXmlSerializer)
         {
            // FUTURE CODE: how do we handle the target namespace here?
            parameter = new SoapOperationParameter(
               (IXmlSerializer)params[count], null);
         }
         else
         {
            parameter = new SoapOperationParameter(
               part.getName(), String.valueOf(params[count]), null); 
         }
         
         // add the parameter
         operation.addParameter(parameter);
      }
      
      return operation;      
   }
   
   /**
    * Gets the WsdlSoapBinding this operation is for.
    * 
    * @return the WsdlSoapBinding this operation is for.
    */
   public WsdlSoapBinding getBinding()
   {
      return mBinding;
   }
   
   /**
    * Sets the port type operation associated with this operation.
    * 
    * @param operation the port type operation associated with this
    *                  operation.
    */
   public void setPortTypeOperation(WsdlPortTypeOperation operation)
   {
      mPortTypeOperation = operation;
   }
   
   /**
    * Gets the port type operation associated with this operation.
    * 
    * @return the port type operation associated with this operation.
    */
   public WsdlPortTypeOperation getPortTypeOperation()
   {
      return mPortTypeOperation;
   }
   
   /**
    * Gets the name of this operation.
    * 
    * @return the name of this operation.
    */
   public String getName()
   {
      return getPortTypeOperation().getName();
   }
   
   /**
    * Sets the soap action associated with this operation.
    * 
    * @param action the soap action associated with this operation.
    */
   public void setSoapAction(String action)
   {
      mSoapAction = action;
   }
   
   /**
    * Gets the soap action associated with this operation.
    * 
    * @return the soap action associated with this operation.
    */
   public String getSoapAction()
   {
      return mSoapAction;
   }
   
   /**
    * Creates a SOAP request SoapOperation.
    * 
    * @param params the parameter values for the operation.
    * 
    * @return the SOAP request SoapOperation.
    */
   public SoapOperation createRequestSoapOperation(Object[] params)
   {
      // get the request message from the port type operation
      WsdlMessage message = getPortTypeOperation().getRequestMessage();
      
      // create soap operation
      SoapOperation operation = createSoapOperation(message, params);
      
      return operation;
   }
   
   /**
    * Creates a SOAP response SoapOperation.
    * 
    * @param params the parameter values for the operation. 
    * 
    * @return the SOAP response SoapOperation.
    */
   public SoapOperation createResponseSoapOperation(Object[] params)
   {
      // get the response message from the port type operation
      WsdlMessage message = getPortTypeOperation().getResponseMessage();
      
      // create soap operation
      SoapOperation operation = createSoapOperation(message, params);
      
      return operation;
   }
   
   /**
    * Gets the parameters for a SoapOperation as an array of objects.
    * 
    * @param operation the SoapOperation.
    * 
    * @return the parameters as an array of objects.
    * 
    * @exception SoapMethodNotRecognizedException thrown if the operation is
    *                                             not recognized.
    */
   public Object[] getParameterArray(SoapOperation operation)
   throws SoapMethodNotRecognizedException
   {
      Object[] params = new Object[operation.getParameters().size()];
      
      // get the wsdl request message
      WsdlMessage message = getPortTypeOperation().getRequestMessage();
      
      // go through the parameters of the operation and parse them as objects
      int count = 0;
      for(Iterator i = operation.getParameters().iterator();
          i.hasNext(); count++)
      {
         SoapOperationParameter p = (SoapOperationParameter)i.next();
         
         // get the message part
         WsdlMessagePart part = message.getParts().getPart(p.getName());
         
         Object parameter = null;
         if(p.isPrimitive())
         {
            parameter = Wsdl.parseObject(p.getValue(), part.getType());
         }
         else
         {
            parameter = getBinding().getWsdl().parseObject(
               p.getValueAsXmlElement(), part.getType());
         }
         
         params[count] = parameter;
      }
      
      return params;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "operation";
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
      
      // create and add soap operation element
      XmlElement soapOperationElement = new XmlElement(
         "operation", "soap", Wsdl.WSDL_SOAP_NAMESPACE_URI);
      soapOperationElement.addAttribute("soapAction", getSoapAction());
      element.addChild(soapOperationElement);
      
      // create input and output elements
      XmlElement inputElement = new XmlElement("input");
      addSoapBodyToXmlElement(inputElement);
      
      XmlElement outputElement = new XmlElement("output");
      addSoapBodyToXmlElement(outputElement);
      
      if(getPortTypeOperation().usesOnlyInputMessage())
      {
         // add input element as a child
         element.addChild(inputElement);
      }
      else if(getPortTypeOperation().usesOnlyOutputMessage())
      {
         // add output element as a child
         element.addChild(outputElement);
      }
      else
      {
         if(getPortTypeOperation().isInputFirst())
         {
            // add input element as a child
            element.addChild(inputElement);

            // add output element as a child
            element.addChild(outputElement);
         }
         else
         {
            // add output element as a child
            element.addChild(outputElement);

            // add input element as a child
            element.addChild(inputElement);
         }
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
      
      // clear soap action
      setSoapAction("");
      
      if(element.getName().equals(getRootTag()))
      {
         // get the soap operation child
         XmlElement soapOperationChild = element.getFirstChild(
            "operation", Wsdl.WSDL_SOAP_NAMESPACE_URI); 
         if(soapOperationChild != null)
         {
            // set soap action
            setSoapAction(soapOperationChild.getAttributeValue("soapAction"));
         
            // FUTURE CODE: current implementation assumes soap encoding
            // parameters -- we'll want to parse these out in the future
         }
         
         rval = true;
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
