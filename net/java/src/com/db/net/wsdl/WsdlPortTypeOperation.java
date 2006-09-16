/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.wsdl;

import java.util.Iterator;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;
   
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
    * The WsdlPortType this operation is associated with.
    */
   protected WsdlPortType mPortType;
   
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
    * 
    * @param portType the WsdlPortType this operation is associated with.
    */
   public WsdlPortTypeOperation(WsdlPortType portType)   
   {
      this(portType, "");
   }
   
   /**
    * Creates a new Wsdl Port Type Operation with the given name.
    * 
    * @param portType the WsdlPortType this operation is associated with.
    * @param name the name of this operation.
    */
   public WsdlPortTypeOperation(WsdlPortType portType, String name)
   {
      // store port type
      mPortType = portType;
      
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
    * Adds the child XML element for the input message.
    * 
    * @param element the element to add the child to.
    */
   protected void addInputMessageXml(XmlElement element)
   {
      // get the input message name
      String name = getInputMessageName();
      
      // get the namespace prefix
      String prefix = element.findNamespacePrefix(
         getPortType().getNamespaceUri());
      if(prefix != null)
      {
         name = prefix + ":" + name;
      }
      
      // create child element
      XmlElement child = new XmlElement("input");
      child.addAttribute("message", name);
      
      // add child
      element.addChild(child);
   }
   
   /**
    * Adds the child XML element for the output message.
    * 
    * @param element the element to add the child to.
    */
   protected void addOutputMessageXml(XmlElement element)
   {
      // get the output message name
      String name = getOutputMessageName();
      
      // get the namespace prefix
      String prefix = element.findNamespacePrefix(
         getPortType().getNamespaceUri());
      if(prefix != null)
      {
         name = prefix + ":" + name;
      }
      
      // create child element
      XmlElement child = new XmlElement("output");
      child.addAttribute("message", name);
      
      // add child
      element.addChild(child);
   }
   
   /**
    * Gets the wsdl port type this operation is associated with.
    * 
    * @return the wsdl port type this operation is associated with.
    */
   public WsdlPortType getPortType()
   {
      return mPortType;
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
    * Gets the WsdlMessage for a request made to a web service.
    * 
    * @return the WsdlMessage for a request made to a web service.
    */
   public WsdlMessage getRequestMessage()
   {
      WsdlMessage rval = null;
      
      // get the message name
      String messageName = "";
      if(usesOnlyInputMessage())
      {
         messageName = getInputMessageName();
      }
      else if(usesOnlyOutputMessage())
      {
         messageName = getOutputMessageName();
      }
      else
      {
         if(isInputFirst())
         {
            messageName = getInputMessageName();
         }
         else
         {
            messageName = getOutputMessageName();
         }
      }
      
      // get the wsdl message
      rval = getPortType().getWsdl().getMessages().getMessage(messageName);
      
      return rval;
   }
   
   /**
    * Gets the WsdlMessage for a response made from a web service.
    * 
    * @return the WsdlMessage for a response made from a web service.
    */
   public WsdlMessage getResponseMessage()
   {
      WsdlMessage rval = null;
      
      // get the message name
      String messageName = "";
      if(usesOnlyInputMessage())
      {
         messageName = getInputMessageName();
      }
      else if(usesOnlyOutputMessage())
      {
         messageName = getOutputMessageName();
      }
      else
      {
         if(isInputFirst())
         {
            messageName = getOutputMessageName();
         }
         else
         {
            messageName = getInputMessageName();
         }
      }
      
      // get the wsdl message
      rval = getPortType().getWsdl().getMessages().getMessage(messageName);
      
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
      element.addAttribute("name", getName());
      
      // add optional parameter order if appropriate
      if(!getParameterOrder().equals(""))
      {
         element.addAttribute("parameterOrder", getParameterOrder());
      }
      
      // message names
      if(usesOnlyInputMessage())
      {
         addInputMessageXml(element);
      }
      else if(usesOnlyOutputMessage())
      {
         addOutputMessageXml(element);
      }
      else
      {
         if(isInputFirst())
         {
            addInputMessageXml(element);
            addOutputMessageXml(element);
         }
         else
         {
            addOutputMessageXml(element);
            addInputMessageXml(element);
         }
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
      
      // clear parameter order
      setParameterOrder("");
      
      // clear input and output messages
      setInputMessageName("");
      setOutputMessageName("");
      
      if(element.getName().equals(getRootTag()))
      {
         // get name
         setName(element.getAttributeValue("name"));
         
         // get parameter order
         setParameterOrder(element.getAttributeValue("parameterOrder"));
         
         // read message names
         boolean inputRead = false;
         for(Iterator i = element.getChildren().iterator(); i.hasNext();)
         {
            XmlElement child = (XmlElement)i.next();
            if(child.getName().equals("input"))
            {
               // get the message name
               setInputMessageName(child.getAttributeValue("message"));
               
               if(!inputRead)
               {
                  inputRead = true;
               }
            }
            else if(child.getName().equals("output"))
            {
               // get the message name
               setOutputMessageName(child.getAttributeValue("message"));
               
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
