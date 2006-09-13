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
 * A WSDL Message.
 * 
 * A WSDL Message defines a message (an input or an output) that some web
 * service can send or receive. It may contain message parts (i.e. like
 * parameters [for input messages] or return values [for output messages])
 * that have a specified type.
 *  
 * @author Dave Longley
 */
public class WsdlMessage extends AbstractXmlSerializer
{
   /**
    * The WSDL this message is associated with.
    */
   protected Wsdl mWsdl;
   
   /**
    * The name of this message.
    */
   protected String mName;
   
   /**
    * The parts for this message.
    */
   protected WsdlMessagePartCollection mPartCollection;
   
   /**
    * Creates a new blank WsdlMessage.
    * 
    * @param wsdl the WSDL this message is associated with.
    */
   public WsdlMessage(Wsdl wsdl)   
   {
      this(wsdl, "");
   }
   
   /**
    * Creates a new WsdlMessage with the given name.
    * 
    * @param wsdl the WSDL this message is associated with.
    * @param name the name of this message.
    */
   public WsdlMessage(Wsdl wsdl, String name)
   {
      // store wsdl
      mWsdl = wsdl;
      
      // store name
      setName(name);
   }
   
   /**
    * Gets the wsdl this message is associated with.
    * 
    * @return the wsdl this message is associated with.
    */
   public Wsdl getWsdl()
   {
      return mWsdl;
   }   
   
   /**
    * Sets the name of this message.
    * 
    * @param name the name of this message.
    */
   public void setName(String name)
   {
      mName = name;
   }
   
   /**
    * Gets the name of this message.
    * 
    * @return the name of this message.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Gets the namespace URI for this message.
    * 
    * @return the namespace URI for this message.
    */
   public String getNamespaceUri()
   {
      return getWsdl().getTargetNamespaceUri();
   }
   
   /**
    * Gets the parts for this message.
    * 
    * @return the parts for this message.
    */
   public WsdlMessagePartCollection getParts()
   {
      if(mPartCollection == null)
      {
         // create parts collection
         mPartCollection = new WsdlMessagePartCollection();
      }
      
      return mPartCollection;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "message";
   }
   
   /**
    * Creates an XmlElement from this object.
    *
    * @param parent the parent XmlElement for the XmlElement being created
    *               (can be null). 
    * 
    * @return the XmlElement that represents this object.
    */
   public XmlElement convertToXmlElement(XmlElement parent)
   {
      // create xml element
      XmlElement element = new XmlElement(getRootTag());
      element.setParent(parent);
      
      // add attributes
      element.getAttributeMap().addAttribute("name", getName());
      
      // parts
      for(Iterator i = getParts().iterator(); i.hasNext();)
      {
         WsdlMessagePart part = (WsdlMessagePart)i.next();
         element.addChild(part.convertToXmlElement(element));
      }
      
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
      
      // clear parts
      getParts().clear();

      if(element.getName().equals(getRootTag()))
      {
         // get name
         setName(element.getAttributeValue("name"));
         
         // read parts
         for(Iterator i = element.getChildren("part").iterator(); i.hasNext();)
         {
            XmlElement child = (XmlElement)i.next();
            WsdlMessagePart part = new WsdlMessagePart();
            if(part.convertFromXmlElement(child))
            {
               // part converted, add it
               getParts().add(part);
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
    * A WSDL message part collection.
    * 
    * @author Dave Longley
    */
   public class WsdlMessagePartCollection
   {
      /**
       * The underlying vector for storing parts. 
       */
      protected Vector mParts;
      
      /**
       * Creates a new WsdlMessagePartCollection.
       */
      public WsdlMessagePartCollection()
      {
         // initialize parts vector
         mParts = new Vector();
      }
      
      /**
       * Adds a new part to this collection.
       * 
       * @param part the part to add to this collection.
       */
      public void add(WsdlMessagePart part)
      {
         mParts.add(part);
      }
      
      /**
       * Removes a part from this collection.
       * 
       * @param part the part to remove from this collection.
       */
      public void remove(WsdlMessagePart part)
      {
         mParts.remove(part);
      }
      
      /**
       * Gets an part from this collection according to its name.
       * 
       * @param name the name of the part to retrieve.
       * 
       * @return the part or null if one was not found.
       */
      public WsdlMessagePart getPart(String name)
      {
         WsdlMessagePart rval = null;
         
         for(Iterator i = iterator(); i.hasNext() && rval == null;) 
         {
            WsdlMessagePart part = (WsdlMessagePart)i.next();
            if(part.getName().equals(name))
            {
               rval = part;
            }
         }
         
         return rval;
      }      
      
      /**
       * Clears all the parts from this collection.
       */
      public void clear()
      {
         mParts.clear();
      }
      
      /**
       * Gets an iterator over the parts in this collection.
       * 
       * @return an iterator over the parts in this collection.
       */
      public Iterator iterator()
      {
         return mParts.iterator();
      }
      
      /**
       * Gets the number of parts in this collection.
       * 
       * @return the number of parts in this collection.
       */
      public int size()
      {
         return mParts.size();
      }
   }
}
