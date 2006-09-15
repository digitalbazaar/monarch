/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A UPnPServiceActionArgumentList is a XML serializable list of
 * UPnPServiceActionArguments.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
 * -----------------------------------------------------------------------
 * <pre>
 * 
 * <actionList>
 *   <action>
 *     <name>actionName</name>
 *     <argumentList>
 *       <argument>
 *         <name>formalParameterName</name>
 *         <direction>in xor out</direction>
 *         <retval />
 *         <relatedStateVariable>stateVariableName</relatedStateVariable>
 *       </argument>
 *      Declarations for other arguments defined by UPnP Forum working
 *      committee (if any) go here
 *     </argumentList>
 *   </action>
 *   Declarations for other actions defined by UPnP Forum working committee
 *   (if any) go here
 *   Declarations for other actions added by UPnP vendor (if any) go here
 * </actionList> 
 * 
 * </pre>
 * 
 * argumentList 
 * Required if and only if parameters are defined for action. (Each action may
 * have >= 0 parameters.) Contains the following sub element(s): 
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
public class UPnPServiceActionArgumentList extends AbstractXmlSerializer
{
   /**
    * The UPnPServiceActionArguments for this list.
    */
   protected Vector mArguments;
   
   /**
    * Creates a new UPnPServiceActionArgumentList.
    */
   public UPnPServiceActionArgumentList()
   {
      // create the argument list
      mArguments = new Vector();
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "argumentList";
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
      // create the root element
      XmlElement listElement = new XmlElement(getRootTag());
      listElement.setParent(parent);
      
      // convert each argument to an xml element child
      for(Iterator i = getArguments().iterator(); i.hasNext();)
      {
         UPnPServiceActionArgument argument =
            (UPnPServiceActionArgument)i.next();
         listElement.addChild(argument.convertToXmlElement(listElement));
      }
      
      // return root element
      return listElement;
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
      boolean rval = true;
      
      // clear argument list
      clear();
      
      // convert arguments
      for(Iterator i = element.getChildren("argument").iterator(); i.hasNext();)
      {
         XmlElement argumentElement = (XmlElement)i.next();
         UPnPServiceActionArgument argument = new UPnPServiceActionArgument();
         if(argument.convertFromXmlElement(argumentElement))
         {
            addArgument(argument);
         }
      }
      
      return rval;
   }
   
   /**
    * Adds a UPnPServiceActionArgument to this list.
    * 
    * @param argument the UPnPServiceActionArgument to add.
    */
   public void addArgument(UPnPServiceActionArgument argument)
   {
      getArguments().add(argument);
   }
   
   /**
    * Removes a UPnPServiceActionArgument from this list.
    * 
    * @param argument the UPnPServiceActionArgument to remove.
    */
   public void removeArgument(UPnPServiceActionArgument argument)
   {
      getArguments().remove(argument);
   }
   
   /**
    * Gets the UPnPServiceActionArguments for this list in a vector.
    * 
    * @return the UPnPServiceActionArguments for this list in a vector.
    */
   public Vector getArguments()
   {
      return mArguments;
   }
   
   /**
    * Clears the arguments from this list.
    */
   public void clear()
   {
      getArguments().clear();
   }
   
   /**
    * Gets the number of arguments in this list.
    * 
    * @return the number of arguments in this list.
    */
   public int getArgumentCount()
   {
      return getArguments().size();
   }
   
   /**
    * Gets the logger for this UPnPServiceActionArgumentList.
    * 
    * @return the logger for this UPnPServiceActionArgumentList.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
