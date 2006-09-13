/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.wsdl;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

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
    * Gets the namespace URI for this binding.
    * 
    * @return the namespace URI for this binding.
    */
   public String getNamespaceUri()
   {
      return getWsdl().getTargetNamespaceUri();
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
    * Creates an XmlElement from this object.
    *
    * @param parent the parent XmlElement for the XmlElement being created
    *               (can be null). 
    * 
    * @return the XmlElement that represents this object.
    */
   public abstract XmlElement convertToXmlElement(XmlElement parent);
   
   /**
    * Converts this object from an XmlElement.
    *
    * @param element the XmlElement to convert from.
    * 
    * @return true if successful, false otherwise.
    */
   public abstract boolean convertFromXmlElement(XmlElement element);   
   
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
