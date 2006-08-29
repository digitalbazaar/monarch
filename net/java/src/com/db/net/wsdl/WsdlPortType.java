/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.wsdl;

import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A WSDL Port Type.
 * 
 * A WSDL Port Type describes a set of operations that use messages to
 * accomplish some tasks for a web service.
 *  
 * @author Dave Longley
 */
public class WsdlPortType extends AbstractXmlSerializer
{
   /**
    * The WSDL this port type is associated with.
    */
   protected Wsdl mWsdl;
   
   /**
    * The name of this port type.
    */
   protected String mName;
   
   /**
    * The operations for this port type.
    */
   protected WsdlPortTypeOperationCollection mOperationCollection;   
   
   /**
    * Creates a new blank WsdlPortType.
    * 
    * @param wsdl the WSDL this port type is associated with.
    */
   public WsdlPortType(Wsdl wsdl)
   {
      this(wsdl, "");
   }
   
   /**
    * Creates a new WsdlPortType with the given name.
    * 
    * @param wsdl the WSDL this port type is associated with.
    * @param name the name of this port type.
    */
   public WsdlPortType(Wsdl wsdl, String name)
   {
      // store wsdl
      mWsdl = wsdl;
      
      // store name and namespace uri
      setName(name);
   }
   
   /**
    * Gets the wsdl this port type is associated with.
    * 
    * @return the wsdl this port type is associated with.
    */
   public Wsdl getWsdl()
   {
      return mWsdl;
   }
   
   /**
    * Sets the name of this port type.
    * 
    * @param name the name of this port type.
    */
   public void setName(String name)
   {
      mName = name;
   }
   
   /**
    * Gets the name of this port type.
    * 
    * @return the name of this port type.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Gets the namespace URI for this port type.
    * 
    * @return the namespace URI for this port type.
    */
   public String getNamespaceUri()
   {
      return getWsdl().getTargetNamespaceUri();
   }   
   
   /**
    * Gets the operations for this port type.
    * 
    * @return the operations for this port type.
    */
   public WsdlPortTypeOperationCollection getOperations()
   {
      if(mOperationCollection == null)
      {
         // create operations collection
         mOperationCollection = new WsdlPortTypeOperationCollection();
      }
      
      return mOperationCollection;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "portType";
   }
   
   /**
    * Creates an XmlElement from this object.
    *
    * @return the XmlElement that represents this object.
    */
   public XmlElement convertToXmlElement()
   {
      // create xml element
      XmlElement element = new XmlElement(getRootTag());
      
      // add attributes
      element.addAttribute("name", getName());
      
      // operations
      for(Iterator i = getOperations().iterator(); i.hasNext();)
      {
         WsdlPortTypeOperation operation = (WsdlPortTypeOperation)i.next();
         element.addChild(operation.convertToXmlElement());
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
      
      // clear name
      setName("");
      
      // clear operations
      getOperations().clear();

      if(element.getName().equals(getRootTag()))
      {
         // get name
         setName(element.getAttributeValue("name"));
         
         // read operations
         for(Iterator i = element.getChildren("operation").iterator();
             i.hasNext();)
         {
            XmlElement child = (XmlElement)i.next();
            WsdlPortTypeOperation operation = new WsdlPortTypeOperation(this);
            if(operation.convertFromXmlElement(child))
            {
               // operation converted, add it
               getOperations().add(operation);
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
    * A WSDL Port Type Operation collection.
    * 
    * @author Dave Longley
    */
   public class WsdlPortTypeOperationCollection
   {
      /**
       * The underlying vector for storing operations. 
       */
      protected Vector mOperations;
      
      /**
       * Creates a new Wsdl Port Type Operation Collection.
       */
      public WsdlPortTypeOperationCollection()
      {
         // initialize operations vector
         mOperations = new Vector();
      }
      
      /**
       * Adds a new operation to this collection.
       * 
       * @param operation the operation to add to this collection.
       */
      public void add(WsdlPortTypeOperation operation)
      {
         mOperations.add(operation);
      }
      
      /**
       * Removes a operation from this collection.
       * 
       * @param operation the operation to remove from this collection.
       */
      public void remove(WsdlPortTypeOperation operation)
      {
         mOperations.remove(operation);
      }
      
      /**
       * Gets an operation from this collection according to its name.
       * 
       * @param name the name of the operation to retrieve.
       * 
       * @return the operation or null if one was not found.
       */
      public WsdlPortTypeOperation getOperation(String name)
      {
         WsdlPortTypeOperation rval = null;
         
         // FIXME: we need to check namespaces
         // strip off the namespace prefix
         name = XmlElement.getBasicName(name);
         
         for(Iterator i = iterator(); i.hasNext() && rval == null;) 
         {
            WsdlPortTypeOperation operation = (WsdlPortTypeOperation)i.next();
            if(operation.getName().equals(name))
            {
               rval = operation;
            }
         }
         
         return rval;
      }
      
      /**
       * Clears all the operation from this collection.
       */
      public void clear()
      {
         mOperations.clear();
      }
      
      /**
       * Gets an iterator over the operation in this collection.
       * 
       * @return an iterator over the operation in this collection.
       */
      public Iterator iterator()
      {
         return mOperations.iterator();
      }
      
      /**
       * Gets the number of operations in this collection.
       * 
       * @return the number of operations in this collection.
       */
      public int size()
      {
         return mOperations.size();
      }
   }   
}
