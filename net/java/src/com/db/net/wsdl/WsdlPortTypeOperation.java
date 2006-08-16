/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.wsdl;

import java.util.Iterator;

import org.w3c.dom.Element;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.ElementReader;
   
/**
 * A WSDL Port Type Operation.
 * 
 * A WSDL Port Type Operation defines an operation (i.e. like a method) that
 * can be performed on a particular web service port.
 * 
 * WSDL operations are grouped together under WSDL Port Types.
 * 
 * A WSDL operation uses WSDL messages that define either:
 * 
 * Only an input for the operation.
 * Only an output for the operation.
 * 
 * An input and an output for the operation. In this case, either the port
 * will send an output message and await for an input message, or the port
 * will receive an input message and respond with an output message. The
 * order of the elements in XML will determine which message is used first.
 *  
 * @author Dave Longley
 */
public class WsdlPortTypeOperation extends AbstractXmlSerializer
{
   /**
    * The name of this operation.
    */
   protected String mName;
   
   /**
    * An optional parameter order.
    */
   protected String mParameterOrder;
   
   /**
    * The input message name for this operation, if any.
    */
   protected String mInputMessageName;
   
   /**
    * The output message name for this operation, if any.
    */
   protected String mOutputMessageName;
   
   /**
    * Whether input->output messaging should be used or output->input
    * messaging should be used.
    * 
    * True if input first then output, false if output first then input.
    */
   protected boolean mInputFirst;
   
   /**
    * Creates a new blank Wsdl Port Type Operation.
    */
   public WsdlPortTypeOperation()   
   {
      this("");
   }
   
   /**
    * Creates a new Wsdl Port Type Operation with the given name.
    * 
    * @param name the name of this operation.
    */
   public WsdlPortTypeOperation(String name)
   {
      // store name
      setName(name);
      
      // default parameter order to blank
      setParameterOrder("");
      
      // default input and output messages to blank
      setInputMessageName("");
      setOutputMessageName("");
      
      // default to using input before output
      setInputFirst(true);
   }
   
   /**
    * Gets the XML for the input message.
    * 
    * @return the XML for the input message.
    */
   protected String getInputMessageXml()
   {
      return "<input message=\"tns:" + getInputMessageName() + "\"/>";
   }
   
   /**
    * Gets the XML for the output message.
    * 
    * @return the XML for the output message.
    */
   protected String getOutputMessageXml()
   {
      return "<output message=\"tns:" + getOutputMessageName() + "\"/>";
   }
   
   /**
    * Sets the name of this operation.
    * 
    * @param name the name of this operation.
    */
   public void setName(String name)
   {
      mName = name;
   }
   
   /**
    * Gets the name of this operation.
    * 
    * @return the name of this operation.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Sets the parameter order of this operation.
    * 
    * @param order the parameter order of this operation.
    */
   public void setParameterOrder(String order)
   {
      mParameterOrder = order;
   }
   
   /**
    * Gets the parameter order of this operation.
    * 
    * @return the parameter order of this operation.
    */
   public String getParameterOrder()
   {
      return mParameterOrder;
   }   
   
   /**
    * Sets the input message name.
    * 
    * @param name the input message name.
    */
   public void setInputMessageName(String name)
   {
      mInputMessageName = name;
   }
   
   /**
    * Gets the input message name.
    * 
    * @return the input message name.
    */
   public String getInputMessageName()   
   {
      return mInputMessageName;
   }
   
   /**
    * Sets the output message name.
    * 
    * @param name the output message name.
    */
   public void setOutputMessageName(String name)
   {
      mOutputMessageName = name;
   }
   
   /**
    * Gets the output message name.
    * 
    * @return the output message name.
    */
   public String getOutputMessageName()
   {
      return mOutputMessageName;
   }
   
   /**
    * Sets whether the input or output message should be used first.
    * 
    * This value will not be used if both input and output message names
    * are not set.
    * 
    * @param inputFirst true to use the input message first (default),
    *                   false to use the output message first.
    */
   public void setInputFirst(boolean inputFirst)
   {
      mInputFirst = inputFirst;
   }
   
   /**
    * Gets whether the input or output message should be used first.
    * 
    * This value will not be used if both input and output message names
    * are not set.
    * 
    * @return true if the input message will be used first (default),
    *         false if the output message will be used first.
    */
   public boolean isInputFirst()
   {
      return mInputFirst;
   }
   
   /**
    * True if this operation uses input and output messages. False otherwise.
    * 
    * @return true if this operation uses input and output messages,
    *         false otherwise.
    */
   public boolean usesInputAndOutputMessages()
   {
      boolean rval = false;
      
      // see if there are input and output message names 
      if(!getInputMessageName().equals("") &&
         !getOutputMessageName().equals(""))      
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * True if this operation uses ONLY an input message. False otherwise.
    * 
    * @return true if this operation uses ONLY input message, false otherwise.
    */
   public boolean usesOnlyInputMessage()   
   {
      boolean rval = false;
      
      // see if there is ONLY an input message 
      if(!getInputMessageName().equals("") && getOutputMessageName().equals(""))      
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * True if this operation uses ONLY an output message. False otherwise.
    * 
    * @return true if this operation uses ONLY output message, false otherwise.
    */
   public boolean usesOnlyOutputMessage()   
   {
      boolean rval = false;
      
      // see if there is ONLY an output message 
      if(getInputMessageName().equals("") && !getOutputMessageName().equals(""))      
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "operation";
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
      
      // add optional parameter order if appropriate
      if(!getParameterOrder().equals(""))
      {
         xml.append("\" parameterOrder=\"");
         xml.append(getParameterOrder());
      }
      
      xml.append("\">");
      
      // message names
      if(usesOnlyInputMessage())
      {
         xml.append(indent);
         xml.append(" " + getInputMessageXml());
      }
      else if(usesOnlyOutputMessage())
      {
         xml.append(indent);
         xml.append(" " + getOutputMessageXml());
      }
      else
      {
         if(isInputFirst())
         {
            xml.append(indent);
            xml.append(" " + getInputMessageXml());
            xml.append(indent);
            xml.append(" " + getOutputMessageXml());
         }
         else
         {
            xml.append(indent);
            xml.append(" " + getOutputMessageXml());
            xml.append(indent);
            xml.append(" " + getInputMessageXml());
         }
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
    * @param element the parsed element that contains this objects
    *                information.
    * 
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXml(Element element)
   {
      boolean rval = false;
      
      // clear name
      setName("");
      
      // clear parameter order
      setParameterOrder("");
      
      // clear input and output messages
      setInputMessageName("");
      setOutputMessageName("");
      
      // get element reader
      ElementReader er = new ElementReader(element);
      if(er.getTagName().equals(getRootTag()))
      {
         // get name
         setName(Wsdl.resolveName(er.getStringAttribute("name")));
         
         // get parameter order
         setParameterOrder(er.getStringAttribute("parameterOrder"));
         
         // read message names
         boolean inputRead = false;
         for(Iterator i = er.getElementReaders().iterator(); i.hasNext();)
         {
            ElementReader reader = (ElementReader)i.next();
            if(reader.getTagName().equals("input"))
            {
               // get the message name
               setInputMessageName(reader.getStringAttribute("message"));
               
               if(!inputRead)
               {
                  inputRead = true;
               }
            }
            else if(reader.getTagName().equals("output"))
            {
               // get the message name
               setOutputMessageName(reader.getStringAttribute("message"));
               
               // set input first based on whether or not it has been read
               setInputFirst(inputRead);
            }
         }
         
         // ensure there is a name and at least one message
         if(!getName().equals("") &&
            (!getInputMessageName().equals("") ||
             !getOutputMessageName().equals("")))
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
