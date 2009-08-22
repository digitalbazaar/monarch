/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.wsdl;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

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
    * Creates the base XmlElement for this port. The port's name and
    * binding will be added as attributes.
    * 
    * @param parent the XmlElement that is the parent of the port XmlElement.
    * 
    * @return the port XmlElement.
    */
   public XmlElement createPortXmlElement(XmlElement parent)
   {
      // create the element
      XmlElement element = new XmlElement("port");
      element.setParent(parent);
      
      // get the binding name
      String name = getBinding().getName();
      
      // get the namespace prefix for the binding
      String prefix = element.findNamespacePrefix(getNamespaceUri());
      if(prefix != null)
      {
         name = prefix + ":" + name;
      }
      
      // add attributes
      element.addAttribute("name", getName());
      element.addAttribute("binding", name);

      // return the element
      return element;
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
    * Gets the namespace URI for this port.
    * 
    * @return the namespace URI for this port.
    */
   public String getNamespaceUri()
   {
      return getBinding().getWsdl().getTargetNamespaceUri();
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   @Override
   public String getRootTag()   
   {
      return "port";
   }
   
   /**
    * Creates an XmlElement from this object.
    *
    * @param parent the parent XmlElement for the XmlElement being created
    *               (can be null). 
    * 
    * @return the XmlElement that represents this object.
    */
   @Override
   public abstract XmlElement convertToXmlElement(XmlElement parent);
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   @Override
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
