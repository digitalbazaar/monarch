/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import org.w3c.dom.Element;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;

/**
 * A WSDL Binding.
 * 
 * A WSDL Binding binds a port type's operations to a particular message
 * format and transmission protocol.
 * 
 * This class is abstract, a class that actually provides a message encoding
 * and tranmission protocol must extend this class to complete it.
 *  
 * @author Dave Longley
 */
public abstract class WsdlBinding extends AbstractXmlSerializer
{
   /**
    * The WSDL this binding is associated with.
    */
   protected Wsdl mWsdl;
   
   /**
    * The port type for this binding.
    */
   protected WsdlPortType mPortType;
   
   /**
    * The name of this binding.
    */
   protected String mName;
   
   /**
    * Creates a new blank WsdlBinding.
    * 
    * @param wsdl the wsdl this binding is associated with.
    */
   public WsdlBinding(Wsdl wsdl)
   {
      this(wsdl, "", null);
   }
   
   /**
    * Creates a new WsdlBinding with the given name.
    * 
    * @param wsdl the wsdl this binding is associated with.
    * @param name the name of this binding.
    * @param portType the port type for this binding.
    */
   public WsdlBinding(Wsdl wsdl, String name, WsdlPortType portType)
   {
      // store wsdl
      mWsdl = wsdl;
      
      // store name and port type
      setName(name);
      setPortType(portType);
   }
   
   /**
    * Gets the wsdl this binding is associated with.
    * 
    * @return the wsdl this binding is associated with.
    */
   public Wsdl getWsdl()
   {
      return mWsdl;
   }
   
   /**
    * Sets the name of this binding.
    * 
    * @param name the name of this binding.
    */
   public void setName(String name)
   {
      mName = name;
   }
   
   /**
    * Gets the name of this binding.
    * 
    * @return the name of this binding.
    */
   public String getName()
   {
      return mName;
   }

   /**
    * Sets the port type for this binding.
    * 
    * @param portType the port type for this binding.
    */
   public void setPortType(WsdlPortType portType)
   {
      mPortType = portType;
   }
   
   /**
    * Gets the port type for this binding.
    * 
    * @return the port type for this binding.
    */
   public WsdlPortType getPortType()
   {
      return mPortType;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "binding";
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
