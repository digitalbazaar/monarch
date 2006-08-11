/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.util.Iterator;
import java.util.Vector;

import org.w3c.dom.Element;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.ElementReader;

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
    * The name of this port type.
    */
   protected String mName;
   
   /**
    * The operations for this port type.
    */
   protected WsdlPortTypeOperationCollection mOperationCollection;   
   
   /**
    * Creates a new blank WsdlPortType.
    */
   public WsdlPortType()
   {
      this("");
   }
   
   /**
    * Creates a new WsdlPortType with the given name.
    * 
    * @param name the name of this port type.
    */
   public WsdlPortType(String name)
   {
      // store name
      setName(name);
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
      
      // operations
      for(Iterator i = getOperations().iterator(); i.hasNext();)
      {
         WsdlPortTypeOperation operation = (WsdlPortTypeOperation)i.next();
         xml.append(operation.convertToXml(indentLevel + 1));
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
      
      // clear operations
      getOperations().clear();

      // get element reader
      ElementReader er = new ElementReader(element);
      if(er.getTagName().equals(getRootTag()))
      {
         // get name
         setName(er.getStringAttribute("name"));
         
         // read operations
         for(Iterator i = er.getElements("operation").iterator(); i.hasNext();)
         {
            Element e = (Element)i.next();
            WsdlPortTypeOperation operation = new WsdlPortTypeOperation();
            if(operation.convertFromXml(e))
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
