/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;
import com.db.xml.XmlException;

/**
 * A UPnPServiceActionArgument is a XML serializable argument for a
 * UPnPServiceAction.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
 * -----------------------------------------------------------------------
 * <pre>
 * 
 * <argument>
 *  <name>formalParameterName</name>
 *  <direction>in xor out</direction>
 *  <retval />
 *  <relatedStateVariable>stateVariableName</relatedStateVariable>
 * </argument>
 * 
 * </pre>
 * 
 * argument 
 * Required. Repeat once for each parameter. Contains the following sub
 * elements: 
 * 
 * name 
 * Required. Name of formal parameter. Should be name of a state variable that
 * models an effect the action causes. Must not contain a hyphen character
 * (-, 2D Hex in UTF-8). String. Should be < 32 characters. 
 * 
 * direction 
 * Required. Whether argument is an input or output parameter. Must be in xor
 * out. Any in arguments must be listed before any out arguments. 
 * 
 * retval 
 * Optional. Identifies at most one out argument as the return value. If
 * included, must be the first out argument. (Element only; no value.) 
 * 
 * relatedStateVariable 
 * Required. Must be the name of a state variable. 
 * -----------------------------------------------------------------------
 * 
 * @author Dave Longley
 */
public class UPnPServiceActionArgument extends AbstractXmlSerializer
{
   /**
    * The name for this argument.
    */
   protected String mName;
   
   /**
    * The direction for the argument (either "in" or "out").
    */
   protected String mDirection;
   
   /**
    * True if this argument is a return value, false if not.
    */
   protected boolean mReturnValue;
   
   /**
    * The name of the state variable this argument applies to.
    */
   protected String mStateVariable;
   
   /**
    * Creates a new UPnPServiceActionArgument.
    */
   public UPnPServiceActionArgument()
   {
      // set defaults
      setName("");
      setDirection(true);
      setReturnValue(false);
      setStateVariable("");
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   @Override
   public String getRootTag()   
   {
      return "argument";
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
      // create the root element
      XmlElement argumentElement = new XmlElement(getRootTag());
      argumentElement.setParent(parent);
      
      // add the name element
      XmlElement nameElement = new XmlElement("name");
      nameElement.setValue(getName());
      argumentElement.addChild(nameElement);
      
      // add the direction element
      XmlElement directionElement = new XmlElement("direction");
      directionElement.setValue(getDirection());
      argumentElement.addChild(directionElement);
      
      // add the return value element, if applicable
      if(isReturnValue())
      {
         XmlElement retvalElement = new XmlElement("retval");
         argumentElement.addChild(retvalElement);
      }
      
      // add the related state variable element
      XmlElement relatedStateVariableElement =
         new XmlElement("relatedStateVariable");
      relatedStateVariableElement.setValue(getStateVariable());
      argumentElement.addChild(relatedStateVariableElement);
      
      // return root element
      return argumentElement;
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
      
      // convert the name element
      setName(element.getFirstChildValue("name"));
      
      // convert the direction element
      setDirection(element.getFirstChildValue("direction").equals("in"));
      
      // convert the return value element
      setReturnValue(element.hasChild("retval"));
      
      // convert the related state variable element
      setStateVariable(element.getFirstChildValue("relatedStateVariable"));
   }
   
   /**
    * Sets the name for this argument. The name will be truncated at 31
    * characters.
    * 
    * @param name the name for this argument.
    */
   public void setName(String name)
   {
      if(name == null)
      {
         name = "";
      }
      else if(name.length() > 31)
      {
         name = name.substring(0, 32);
      }
      
      mName = name;
   }
   
   /**
    * Gets the name for this argument.
    * 
    * @return the name for this argument.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Sets the direction for the argument (either "in" or "out").
    * 
    * @param in true to set the direction to "in", false to set it to "out."
    */
   public void setDirection(boolean in)
   {
      if(in)
      {
         mDirection = "in";
      }
      else
      {
         mDirection = "out";
      }
   }
   
   /**
    * Sets the direction for the argument (either "in" or "out").
    * 
    * @return the direction for the argument.
    */
   public String getDirection()
   {
      return mDirection;
   }
   
   /**
    * Returns true if this argument's direction is "in," false if it is "out."
    * 
    * @return true if this argument's direction is "in," false if it is "out."
    */
   public boolean isDirectionIn()
   {
      return mDirection.equals("in");
   }
   
   /**
    * Sets whether or not this argument is a return value.
    * 
    * @param rval true if this argument is a return value, false if not.
    */
   public void setReturnValue(boolean rval)
   {
      mReturnValue = rval;
      
      if(rval)
      {
         // direction must be "out" if this argument is a return value
         setDirection(false);
      }
   }
   
   /**
    * Gets whether or not this argument is a return value.
    * 
    * @return true if this argument is a return value, false if not.
    */
   public boolean isReturnValue()
   {
      return mReturnValue;
   }
   
   /**
    * Sets the name of the state variable this argument applies to.
    * 
    * @param name the name of the state variable this argument applies to.
    */
   public void setStateVariable(String name)
   {
      mStateVariable = name;
   }
   
   /**
    * Gets the name of the state variable this argument applies to.
    * 
    * @return the name of the state variable this argument applies to.
    */
   public String getStateVariable()
   {
      return mStateVariable;
   }
   
   /**
    * Gets the logger for this UPnPServiceActionArgument.
    * 
    * @return the logger for this UPnPServiceActionArgument.
    */
   @Override
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
