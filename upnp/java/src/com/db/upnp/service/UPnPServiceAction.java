/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A UPnPServiceAction is an XML serializable action that a UPnPService can
 * perform.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
 * -----------------------------------------------------------------------
 * <pre>
 * 
 * <action>
 *   <name>actionName</name>
 *   <argumentList>
 *     <argument>
 *       <name>formalParameterName</name>
 *       <direction>in xor out</direction>
 *       <retval />
 *       <relatedStateVariable>stateVariableName</relatedStateVariable>
 *     </argument>
 *    Declarations for other arguments defined by UPnP Forum working
 *    committee (if any) go here
 *   </argumentList>
 * </action>
 * 
 * </pre>
 * 
 * action 
 * Required. Repeat once for each action defined by a UPnP Forum working
 * committee. If UPnP vendor differentiates service by adding additional
 * actions, repeat once for each additional action. Contains the following
 * sub elements: 
 * 
 * name 
 * Required. Name of action. Must not contain a hyphen character (-, 2D Hex in
 * UTF-8) nor a hash character (#, 23 Hex in UTF-8).  
 * 
 * For standard actions defined by a UPnP Forum working committee, must not
 * begin with X_ nor A_.
 * 
 * For non-standard actions specified by a UPnP vendor and added to a standard
 * service, must begin with X_.
 * 
 * String. Should be < 32 characters. 
 * 
 * argumentList 
 * Required if and only if parameters are defined for action. (Each action may
 * have >= 0 parameters.) Contains the following sub element(s): 
 * -----------------------------------------------------------------------
 * 
 * @author Dave Longley
 */
public class UPnPServiceAction extends AbstractXmlSerializer
{
   /**
    * The name for this action.
    */
   protected String mName;
   
   /**
    * The UPnPServiceActionArgumentList for this action, if this action has
    * arguments.
    */
   protected UPnPServiceActionArgumentList mArgumentList;
   
   /**
    * Creates a new UPnPServiceAction.
    */
   public UPnPServiceAction()
   {
      // set defaults
      setName("");

      // create an empty argument list
      mArgumentList = new UPnPServiceActionArgumentList();
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "action";
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
      XmlElement actionElement = new XmlElement(getRootTag());
      actionElement.setParent(parent);
      
      // add the name element
      XmlElement nameElement = new XmlElement(getName());
      actionElement.addChild(nameElement);
      
      // see if there are arguments for this action
      if(getArgumentList().getArgumentCount() > 0)
      {
         // add argument list element
         XmlElement argumentListElement =
            getArgumentList().convertToXmlElement(actionElement);
         actionElement.addChild(argumentListElement);
      }
      
      // return root element
      return actionElement;
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
      
      // clear this action's name and arguments
      setName("");
      getArgumentList().clear();
      
      // get name element
      XmlElement nameElement = element.getFirstChild("name");
      setName(nameElement.getValue());
      
      // get the argument list, if any
      XmlElement argumentListElement = element.getFirstChild("argumentList");
      if(argumentListElement != null)
      {
         rval = getArgumentList().convertFromXmlElement(argumentListElement);
      }
      
      return rval;
   }
   
   /**
    * Sets the name for this action. The name will be truncated at 31
    * characters.
    * 
    * @param name the name for this action.
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
    * Gets the name for this action.
    * 
    * @return the name for this action.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Gets the argument list for this action.
    * 
    * @return the UPnPServiceActionArgumentList for this action.
    */
   public UPnPServiceActionArgumentList getArgumentList()
   {
      return mArgumentList;
   }
   
   /**
    * Gets the logger for this UPnPServiceAction.
    * 
    * @return the logger for this UPnPServiceAction.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
