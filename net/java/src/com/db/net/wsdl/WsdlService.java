/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.wsdl;

import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.soap.WsdlSoapPort;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;
import com.db.xml.XmlException;

/**
 * A WSDL Service.
 * 
 * A WSDL Service describes the set of ports that web services are provided
 * over. 
 * 
 * @author Dave Longley
 */
public class WsdlService extends AbstractXmlSerializer
{
   /**
    * The WSDL this service is associated with.
    */
   protected Wsdl mWsdl;
   
   /**
    * The name of this service.
    */
   protected String mName;
   
   /**
    * The Wsdl Ports for this service.
    */
   protected WsdlPortCollection mPortCollection;
   
   /**
    * Creates a new blank WsdlService.
    * 
    * @param wsdl the wsdl this service is associated with.
    */
   public WsdlService(Wsdl wsdl)
   {
      this(wsdl, "");
   }   
   
   /**
    * Creates a new WsdlService with the given name.
    * 
    * @param wsdl the wsdl this service is associated with.
    * @param name the name of the service.
    */
   public WsdlService(Wsdl wsdl, String name)
   {
      // store wsdl
      mWsdl = wsdl;
      
      // set name
      setName(name);
   }
   
   /**
    * Gets the wsdl this service is associated with.
    * 
    * @return the wsdl this service is associated with.
    */
   public Wsdl getWsdl()
   {
      return mWsdl;
   }
   
   /**
    * Sets the name of this service.
    * 
    * @param name the name of this service.
    */
   public void setName(String name)
   {
      mName = name;
   }
   
   /**
    * Gets the name of this service.
    * 
    * @return the name of this service.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Gets the namespace URI for this service.
    * 
    * @return the namespace URI for this service.
    */
   public String getNamespaceUri()
   {
      return getWsdl().getTargetNamespaceUri();
   }
   
   /**
    * Gets the ports for this service.
    * 
    * @return the ports for this service.
    */
   public WsdlPortCollection getPorts()
   {
      if(mPortCollection == null)
      {
         // create the port collection for this service
         mPortCollection = new WsdlPortCollection();
      }
      
      return mPortCollection;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   @Override
   public String getRootTag()   
   {
      return "service";
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
   public XmlElement convertToXmlElement(XmlElement parent)
   {
      // create xml element
      XmlElement element = new XmlElement(getRootTag());
      element.setParent(parent);

      // add attributes
      element.addAttribute("name", getName());
      
      // ports
      for(WsdlPort port: getPorts())
      {
         element.addChild(port.convertToXmlElement(element));
      }
      
      // return element
      return element;      
   }
   
   /**
    * Converts this object from an XmlElement.
    *
    * @param element the XmlElement to convert from.
    * 
    * @exception XmlException thrown if this object could not be converted from
    *                         xml.
    */
   @Override
   public void convertFromXmlElement(XmlElement element) throws XmlException
   {
      super.convertFromXmlElement(element);
      
      // clear name
      setName("");
      
      // clear ports
      getPorts().clear();

      // get name
      setName(element.getAttributeValue("name"));

      // read ports
      for(XmlElement child: element.getChildren("port"))
      {
         // get binding
         String bindingName = child.getAttributeValue("binding");
         WsdlBinding binding =
            getWsdl().getBindings().getBinding(bindingName);
         if(binding != null)
         {
            // FUTURE CODE: current implementation can only read
            // WsdlSoapPorts
            XmlElement soapElement =
               child.getFirstChild("address", Wsdl.WSDL_SOAP_NAMESPACE_URI);
            if(soapElement != null)
            {
               WsdlPort port = new WsdlSoapPort(binding);
               port.convertFromXmlElement(child);
               getPorts().add(port);
            }
         }
      }
      
      // ensure there is a name
      if(getName().equals(""))            
      {
         throw new XmlException("No service name!");
      }         
   }
   
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
   
   /**
    * A WSDL Port collection.
    * 
    * @author Dave Longley
    */
   public class WsdlPortCollection implements Iterable<WsdlPort>
   {
      /**
       * The underlying vector for storing operations. 
       */
      protected Vector<WsdlPort> mPorts;
      
      /**
       * Creates a new Wsdl Port Collection.
       */
      public WsdlPortCollection()
      {
         // initialize ports vector
         mPorts = new Vector<WsdlPort>();
      }
      
      /**
       * Adds a new port to this collection.
       * 
       * @param port the operation to add to this collection.
       */
      public void add(WsdlPort port)
      {
         mPorts.add(port);
      }
      
      /**
       * Removes a port from this collection.
       * 
       * @param port the port to remove from this collection.
       */
      public void remove(WsdlPort port)
      {
         mPorts.remove(port);
      }
      
      /**
       * Gets an port from this collection according to its name.
       * 
       * @param name the name of the port to retrieve.
       * 
       * @return the port or null if one was not found.
       */
      public WsdlPort getPort(String name)
      {
         WsdlPort rval = null;
         
         for(WsdlPort port: this) 
         {
            if(port.getName().equals(name))
            {
               rval = port;
               break;
            }
         }
         
         return rval;
      }
      
      /**
       * Clears all the port from this collection.
       */
      public void clear()
      {
         mPorts.clear();
      }
      
      /**
       * Gets an iterator over the port in this collection.
       * 
       * @return an iterator over the port in this collection.
       */
      public Iterator<WsdlPort> iterator()
      {
         return mPorts.iterator();
      }
   }   
}
