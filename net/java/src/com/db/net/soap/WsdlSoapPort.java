/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import org.w3c.dom.Element;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.wsdl.Wsdl;
import com.db.net.wsdl.WsdlBinding;
import com.db.net.wsdl.WsdlPort;
import com.db.xml.ElementReader;

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
      xml.append(getPortOpeningTagXml());
      
      // soap address
      xml.append(indent);
      xml.append(
         "<soap:address location=\"" + getUri() + "\" " +
         "xmlns=\"" + Wsdl.WSDL_NAMESPACE + "\"/>");
      
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
    * @param element the parsed element that contains this objects information.
    * 
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXml(Element element)
   {
      boolean rval = false;
      
      // clear name
      setName("");
      
      // clear uri
      setUri("");
      
      // get element reader
      ElementReader er = new ElementReader(element);
      if(er.getTagName().equals(getRootTag()))
      {
         // get name
         setName(er.getStringAttribute("name"));
         
         // get reader for soap address
         ElementReader reader = er.getFirstElementReader("soap:address");
         if(reader != null)
         {
            setUri(reader.getStringAttribute("location"));
            
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
