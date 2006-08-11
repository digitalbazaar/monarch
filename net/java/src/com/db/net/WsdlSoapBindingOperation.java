/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import org.w3c.dom.Element;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.ElementReader;

/**
 * A WSDL SOAP Binding Operation.
 * 
 * A WSDL SOAP Binding Operation defines the message encoding and 
 * transmission protocol for a WSDL Port Type operation. It indicates
 * that messages will be encoded in a SOAP envelope and sent over HTTP. 
 * 
 * @author Dave Longley
 */
public class WsdlSoapBindingOperation extends AbstractXmlSerializer
{
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
   public static final String SOAP_ENCODING_NAMESPACE =
      "http://schemas.xmlsoap.org/soap/encoding/";   
   
   /**
    * Creates a new Wsdl Soap Binding Operation with the given
    * associated port type operation.
    * 
    * @param operation the associated port type operation.
    */
   public WsdlSoapBindingOperation(WsdlPortTypeOperation operation)
   {
      // store port type operation
      setPortTypeOperation(operation);
      
      // set blank soap action
      setSoapAction("");
   }
   
   /**
    * Gets the SOAP encoding XML.
    * 
    * @return the SOAP encoding XML.
    */
   protected String getSoapEncodingXml()
   {
      return "<soap:body encodingStyle=\"" + SOAP_ENCODING_NAMESPACE +
         "\" use=\"encoded\"/>";
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
    * Sets the port type operation associated with this operation.
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
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "operation";
   }
   
   /**
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    *
    * @param indentLevel the number of spaces to place before the text
    *                    after each new line.
    *                    
    * @return the xml-based representation of the object.
    */
   public String convertToXml(int indentLevel)
   {
      StringBuffer xml = new StringBuffer();
      
      // build indent string
      StringBuffer indent = new StringBuffer("\n");
      for(int i = 0; i < indentLevel; i++)
      {
         indent.append(' ');
      }

      // start tag
      xml.append(indent);
      xml.append('<');
      xml.append(getRootTag());
      xml.append(" name=\"");
      xml.append(getName());
      xml.append("\">");
      
      // soap action
      xml.append(indent);
      xml.append(
         " <soap:operation soapAction=\"" + getSoapAction() + "\"/>");
      
      if(getPortTypeOperation().usesOnlyInputMessage())
      {
         // input
         xml.append(indent);
         xml.append(" <input>");
         xml.append(indent);
         xml.append("  " + getSoapEncodingXml());
         xml.append(indent);
         xml.append(" </input>");
      }
      else if(getPortTypeOperation().usesOnlyOutputMessage())
      {
         // output
         xml.append(indent);
         xml.append(" <output>");
         xml.append(indent);
         xml.append("  " + getSoapEncodingXml());
         xml.append(indent);
         xml.append(" </output>");         }
      else
      {
         if(getPortTypeOperation().isInputFirst())
         {
            // input
            xml.append(indent);
            xml.append(" <input>");
            xml.append(indent);
            xml.append("  " + getSoapEncodingXml());
            xml.append(indent);
            xml.append(" </input>");
            
            // output
            xml.append(indent);
            xml.append(" <output>");
            xml.append(indent);
            xml.append("  " + getSoapEncodingXml());
            xml.append(indent);
            xml.append(" </output>");               
         }
         else
         {
            // output
            xml.append(indent);
            xml.append(" <output>");
            xml.append(indent);
            xml.append("  " + getSoapEncodingXml());
            xml.append(indent);
            xml.append(" </output>");
            
            // input
            xml.append(indent);
            xml.append(" <input>");
            xml.append(indent);
            xml.append("  " + getSoapEncodingXml());
            xml.append(indent);
            xml.append(" </input>");               
         }
      }
      
      // end tag
      xml.append(indent);
      xml.append("</");
      xml.append(getRootTag());
      xml.append('>');
      
      return xml.toString();
   }
   
   /**
    * This method takes a parsed DOM XML element and converts it
    * back into this object's representation.
    *
    * @param element the parsed element that contains this objects
    *                information.
    * 
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXml(Element element)
   {
      boolean rval = false;
      
      // clear soap action
      setSoapAction("");
      
      // get element reader
      ElementReader er = new ElementReader(element);
      if(er.getTagName().equals(getRootTag()))
      {
         // get the soap operation element reader
         ElementReader reader = er.getFirstElementReader("soap:operation");
         
         // set soap action
         setSoapAction(reader.getStringAttribute("soapAction"));
         
         // FUTURE CODE: current implementation assumes soap encoding
         // parameters -- we'll want to parse these out in the future
         
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
