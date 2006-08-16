/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.wsdl;

import java.util.Iterator;
import java.util.Vector;

import org.w3c.dom.Element;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.soap.WsdlSoapPort;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.ElementReader;

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
    * @param wsdl the wsdl this binding is associated with.
    */
   public WsdlService(Wsdl wsdl)
   {
      this(wsdl, "");
   }   
   
   /**
    * Creates a new WsdlService with the given name.
    * 
    * @param wsdl the wsdl this binding is associated with.
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
   public String getRootTag()   
   {
      return "service";
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
      
      // ports
      for(Iterator i = getPorts().iterator(); i.hasNext();)
      {
         WsdlPort port = (WsdlPort)i.next();
         xml.append(port.convertToXml(indentLevel + 1));
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
    * @param element the parsed element that contains this objects information.
    * 
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXml(Element element)
   {
      boolean rval = false;
      
      // clear name
      setName("");
      
      // clear ports
      getPorts().clear();

      // get element reader
      ElementReader er = new ElementReader(element);
      if(er.getTagName().equals(getRootTag()))
      {
         // get name
         setName(er.getStringAttribute("name"));

         // read ports
         for(Iterator i = er.getElementReaders("port").iterator(); i.hasNext();)
         {
            ElementReader reader = (ElementReader)i.next();
            
            // get binding
            String bindingName = reader.getStringAttribute("binding");
            WsdlBinding binding =
               getWsdl().getBindings().getBinding(bindingName);
            if(binding != null)
            {
               // FUTURE CODE: current implementation can only read
               // WsdlSoapPorts
               ElementReader soapReader =
                  reader.getFirstElementReader("soap:address");
               if(soapReader != null)
               {
                  WsdlPort port = new WsdlSoapPort(binding);
                  if(port.convertFromXml(reader.getElement()))
                  {
                     // port converted, add it
                     getPorts().add(port);
                  }
               }
            }
         }

         // ensure there is a name
         if(!getName().equals(""))            
         {
            // conversion successful
            rval = true;
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
    * A WSDL Port collection.
    * 
    * @author Dave Longley
    */
   public class WsdlPortCollection
   {
      /**
       * The underlying vector for storing operations. 
       */
      protected Vector mPorts;
      
      /**
       * Creates a new Wsdl Port Collection.
       */
      public WsdlPortCollection()
      {
         // initialize ports vector
         mPorts = new Vector();
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
         
         for(Iterator i = iterator(); i.hasNext() && rval == null;) 
         {
            WsdlPort port = (WsdlPort)i.next();
            if(port.getName().equals(name))
            {
               rval = port;
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
      public Iterator iterator()
      {
         return mPorts.iterator();
      }
   }   
}
