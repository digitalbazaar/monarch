/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.wsdl;

import org.w3c.dom.Element;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;

/**
 * A WSDL Port.
 * 
 * A WSDL Port describes a location that provides operations for a web service.
 * 
 * This class is abstract. An extending class must complete this class by
 * providing the address for the web service according to the appropriate
 * protocol.
 *  
 * @author Dave Longley
 */
public abstract class WsdlPort extends AbstractXmlSerializer
{
   /**
    * The binding for this port.
    */
   protected WsdlBinding mBinding;
   
   /**
    * The name of this port.
    */
   protected String mName;
   
   /**
    * Creates a new blank WsdlPort.
    * 
    * @param binding the binding for this port.
    */
   public WsdlPort(WsdlBinding binding)
   {
      this(binding, "");
   }
   
   /**
    * Creates a new WsdlPort with the given name.
    * 
    * @param binding the binding for this port.
    * @param name the name of this port.
    */
   public WsdlPort(WsdlBinding binding, String name)
   {
      // store binding
      mBinding = binding;
      
      // store name
      setName(name);
   }

   /**
    * Gets the port XML. This will return the opening tag for the port.
    * 
    * @return the port XML.
    */
   public String getPortOpeningTagXml()
   {
      return "<port name=\"" + getName() + "\" binding=\"" +
         "tns:" + getBinding().getName() + "\">";
   }
   
   /**
    * Gets the binding for this port.
    * 
    * @return the binding for this port.
    */
   public WsdlBinding getBinding()
   {
      return mBinding;
   }

   /**
    * Sets the name of this port.
    * 
    * @param name the name of this port.
    */
   public void setName(String name)
   {
      mName = name;
   }
   
   /**
    * Gets the name of this port.
    * 
    * @return the name of this port.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "port";
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
   public abstract String convertToXml(int indentLevel);
   
   /**
    * This method takes a parsed DOM XML element and converts it
    * back into this object's representation.
    *
    * @param element the parsed element that contains this objects information.
    * 
    * @return true if successful, false otherwise.
    */
   public abstract boolean convertFromXml(Element element);
   
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
