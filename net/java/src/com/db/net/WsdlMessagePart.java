/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import org.w3c.dom.Element;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.ElementReader;
   
/**
 * A WSDL message part. A message part is like a parameter (for an input
 * message) or a return value (for an output message).
 * 
 * The part has a type associated with it.
 * 
 * @author Dave Longley
 */
public class WsdlMessagePart extends AbstractXmlSerializer
{
   /**
    * The name of this part.
    */
   protected String mName;
   
   /**
    * The type of this part.
    */
   protected String mType;

   /**
    * Creates a new blank WsdlMessagePart. 
    */
   public WsdlMessagePart()
   {
      this("", "");
   }
   
   /**
    * Creates a new WsdlMessagePart. 
    * 
    * @param name the name of the part.
    * @param type the type of the part.
    */
   public WsdlMessagePart(String name, String type)
   {
      setName(name);
      setType(type);
   }
   
   /**
    * Sets this part's name.
    * 
    * @param name the name of this part. 
    */
   public void setName(String name)
   {
      mName = name;
   }
   
   /**
    * Gets this part's name.
    * 
    * @return the name of this part. 
    */
   public String getName()
   {
      return mName;
   }      
   
   /**
    * Sets the type of this part.
    * 
    * @param type the type of this part.
    */
   public void setType(String type)
   {
      mType = type;
   }
   
   /**
    * Gets the type of this part.
    * 
    * @return the type of this part.
    */
   public String getType()
   {
      return mType;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "part";
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

      // start and end tag
      xml.append(indent);
      xml.append('<');
      xml.append(getRootTag());
      xml.append(" name=\"");
      xml.append(getName());
      xml.append("\" type=\"");
      xml.append(getType());
      xml.append("\"/>");
      
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
      
      // clear part name and part type
      setName("");
      setType("");
      
      // get element reader
      ElementReader er = new ElementReader(element);
      if(er.getTagName().equals(getRootTag()))
      {
         // get part name
         setName(er.getStringAttribute("name"));
         
         // get part type
         setType(er.getStringAttribute("type"));
         
         // ensure there is a part name and type
         if(!getName().equals("") && !getType().equals(""))            
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
}
