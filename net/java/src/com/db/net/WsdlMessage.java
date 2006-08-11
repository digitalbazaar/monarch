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
    * The name of this message.
    */
   protected String mName;
   
   /**
    * The parts for this message.
    */
   protected WsdlMessagePartCollection mPartCollection;
   
   /**
    * Creates a new blank WsdlMessage.
    */
   public WsdlMessage()   
   {
      this("");
   }
   
   /**
    * Creates a new WsdlMessage with the given name.
    * 
    * @param name the name of this message.
    */
   public WsdlMessage(String name)
   {
      // store name
      setName(name);
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
      
      // parts
      for(Iterator i = getParts().iterator(); i.hasNext();)
      {
         WsdlMessagePart part = (WsdlMessagePart)i.next();
         xml.append(part.convertToXml(indentLevel + 1));
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
      
      // clear parts
      getParts().clear();

      // get element reader
      ElementReader er = new ElementReader(element);
      if(er.getTagName().equals(getRootTag()))
      {
         // get name
         setName(Wsdl.resolveName(er.getStringAttribute("name")));
         
         // read parts
         for(Iterator i = er.getElements("part").iterator(); i.hasNext();)
         {
            Element e = (Element)i.next();
            WsdlMessagePart part = new WsdlMessagePart();
            if(part.convertFromXml(e))
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
