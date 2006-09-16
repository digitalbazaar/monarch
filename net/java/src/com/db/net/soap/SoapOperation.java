/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A SoapOperation is an operation that is performed over SOAP (Simple Object
 * Access Protocol). It has a target namespace URI (a location that defines
 * the XML elements of the operation), a name, a list of parameters, and an
 * encoding style.
 * 
 * A SoapOperation may contain input or output. This means that the operation
 * may be analogous to either a method call or its return value.
 * 
 * A SoapOperation is used in either a SOAP request or a SOAP response. In
 * either case a SoapEnvelope object contains a SoapOperation that defines
 * the request or the response operation and its parameters.
 *
 * SOAP requests are usually made by a client that is communicating with a
 * server. It is used to execute a remote method and receive a response.
 * 
 * SOAP responses are usually made by a server that is communicating with a
 * a client. It is used after a remote method has been executed to return
 * a response with the result from the method as a SoapOperation. In most
 * cases, the SoapOperation will have one parameter which will be the return
 * value from the remote method.
 * 
 * This operation can be converted to and from XML for SOAP transportation.
 * 
 * FUTURE CODE: When we move over to SOAP 1.2 or (even before that if we
 * add complex object support) we want a clean redesign of the soap classes
 * and interfaces to make it more streamlined and easy to use. This may
 * include created a new interface for serializing complex objects to xml.  
 * 
 * @author Dave Longley
 */
public class SoapOperation extends AbstractXmlSerializer
{
   /**
    * The name for this operation.
    */
   protected String mName;
   
   /**
    * The target namespace URI that defines the XML elements of this operation.
    */
   protected String mTargetNamespaceUri;
   
   /**
    * The encoding style URI for this operation.
    */
   protected String mEncodingStyleUri;
   
   /**
    * The SoapOperationParameters for this SoapOperation stored in a vector.
    */
   protected Vector mParameters;
   
   /**
    * Creates a new blank SoapOperation that uses no set name and the
    * default encoding style of SoapEnvelope.SOAP_ENCODING_URI. The name
    * and namespace for this operation must be read from XML.
    */
   public SoapOperation()
   {
      this("", null, SoapEnvelope.SOAP_ENCODING_URI);
   }
   
   /**
    * Creates a new SoapOperation that uses the default encoding style
    * of SoapEnvelope.SOAP_ENCODING_URI.
    * 
    * @param name the name of this operation.
    * @param tns the target namespace URI for this operation.
    */
   public SoapOperation(String name, String tns)
   {
      this(name, tns, SoapEnvelope.SOAP_ENCODING_URI);
   }
   
   /**
    * Creates a new SoapOperation.
    * 
    * @param name the name of this operation.
    * @param tns the target namespace URI for this operation.
    * @param encodingStyle the encoding style URI for this operation.
    */
   public SoapOperation(String name, String tns, String encodingStyle)
   {
      // store the name, target namespace URI, and encoding type
      mName = name;
      mTargetNamespaceUri = tns;
      mEncodingStyleUri = encodingStyle;
      
      // create the parameters vector
      mParameters = new Vector();
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      // return the name of the operation
      return mName;
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
      XmlElement operationElement =
         new XmlElement(getRootTag(), mTargetNamespaceUri);
      operationElement.setParent(parent);
      
      // add an attribute defining the namespace
      operationElement.addAttribute("xmlns:tns", mTargetNamespaceUri);
      
      // add an encoding style attribute (in the SOAP envelope namespace)
      if(mEncodingStyleUri != null && mEncodingStyleUri.length() > 0)
      {
         operationElement.addAttribute(
            "encodingStyle", mEncodingStyleUri, SoapEnvelope.SOAP_ENVELOPE_URI);
      }
      
      // add each parameter
      for(Iterator i = mParameters.iterator(); i.hasNext();)
      {
         SoapOperationParameter parameter = (SoapOperationParameter)i.next();
         operationElement.addChild(
            parameter.convertToXmlElement(operationElement));
      }
      
      // return operation element
      return operationElement;
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
      
      // get the operation name
      mName = element.getName();
      
      // get the target namespace uri
      mTargetNamespaceUri = element.getNamespaceUri();
      
      // get the encoding style
      mEncodingStyleUri = element.getAttributeValue(
         "encodingStyle", SoapEnvelope.SOAP_ENVELOPE_URI);
      
      // convert the parameters
      int count = 0;
      for(Iterator i = element.getChildren().iterator(); i.hasNext(); count++)
      {
         XmlElement parameterElement = (XmlElement)i.next();
         
         // get the parameter to convert with
         SoapOperationParameter parameter = null;
         if(count < mParameters.size())
         {
            parameter = (SoapOperationParameter)mParameters.get(count);
         }
         else
         {
            // create a new parameter that uses an XmlElement to convert
            parameter = new SoapOperationParameter(new XmlElement(), null);
            mParameters.add(parameter);
         }
         
         // convert the parameter from xml
         parameter.convertFromXmlElement(parameterElement);
      }
      
      return rval;
   }
   
   /**
    * Gets the name of this operation.
    * 
    * @return the name of this operation.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Gets the target namespace URI for this operation.
    * 
    * @return the target namespace URI for this operation.
    */
   public String getTargetNamespace()
   {
      return mTargetNamespaceUri;
   }
   
   /**
    * Adds a parameter to this operation.
    * 
    * @param parameter the SoapOperationParameter to add.
    */
   public void addParameter(SoapOperationParameter parameter)
   {
      mParameters.add(parameter);
   }
   
   /**
    * Gets the parameters for this operation in a vector.
    * 
    * @return a vector of SoapOperationParameters.
    */
   public Vector getParameters()
   {
      return mParameters;
   }
   
   /**
    * Returns true if this operation has parameters, false if not.
    * 
    * @return true if this operation has parameters, false if not.
    */
   public boolean hasParameters()
   {
      return getParameters().size() > 0;
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
