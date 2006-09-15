/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

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
    * Creates a new UPnPServiceActionArgumentList.
    */
   public UPnPServiceActionArgumentList()
   {
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "root";
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
      XmlElement scpdElement = new XmlElement(getRootTag());
      scpdElement.setParent(parent);
      
      // FIXME:
      
      // return root element
      return scpdElement;
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

      // FIXME:
      
      return rval;
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
