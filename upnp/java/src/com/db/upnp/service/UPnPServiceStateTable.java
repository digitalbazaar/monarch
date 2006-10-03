/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A UPnPServiceStateTable is an XML serializable table for
 * UPnPServiceStateVariables.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
 * -----------------------------------------------------------------------
 * <pre>
 * 
 * <serviceStateTable>
 *   <stateVariable sendEvents="yes">
 *     <name>variableName</name>
 *     <dataType>variable data type</dataType>
 *     <defaultValue>default value</defaultValue>
 *     <allowedValueList>
 *       <allowedValue>enumerated value</allowedValue>
 *       Other allowed values defined by UPnP Forum working committee
 *       (if any) go here
 *     </allowedValueList>
 *   </stateVariable>
 *   <stateVariable sendEvents="yes">
 *     <name>variableName</name>
 *     <dataType>variable data type</dataType>
 *     <defaultValue>default value</defaultValue>
 *     <allowedValueRange>
 *       <minimum>minimum value</minimum>
 *       <maximum>maximum value</maximum>
 *       <step>increment value</step>
 *     </allowedValueRange>
 *   </stateVariable>
 *   Declarations for other state variables defined by UPnP Forum working
 *   committee (if any) go here
 *   Declarations for other state variables added by UPnP vendor (if any)
 *   go here
 * </serviceStateTable>
 * 
 * </pre>
 * 
 * serviceStateTable 
 * Required. (Each service must have > 0 state variables.) Contains the
 * following sub element(s): 
 * 
 * stateVariable 
 * Required. Repeat once for each state variable defined by a UPnP Forum
 * working committee. If UPnP vendor differentiates service by adding
 * additional state variables, repeat once for each additional variable.
 * sendEvents attribute defines whether event messages will be generated when
 * the value of this state variable changes; non-evented state variables have
 * sendEvents="no"; default is sendEvents="yes".
 * -----------------------------------------------------------------------
 * 
 * @author Dave Longley
 */
public class UPnPServiceStateTable extends AbstractXmlSerializer
implements Iterable<UPnPServiceStateVariable>
{
   /**
    * A hash map of state variable names to state variables.
    */
   protected HashMap<String, UPnPServiceStateVariable> mVariables;
   
   /**
    * Creates a new UPnPServiceStateTable.
    */
   public UPnPServiceStateTable()
   {
      // create the variables map
      mVariables = new HashMap<String, UPnPServiceStateVariable>();
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   @Override
   public String getRootTag()   
   {
      return "serviceStateTable";
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
      XmlElement serviceStateTableElement = new XmlElement(getRootTag());
      serviceStateTableElement.setParent(parent);
      
      // convert and add each state variable
      for(UPnPServiceStateVariable variable: this)
      {
         serviceStateTableElement.addChild(variable.convertToXmlElement(
            serviceStateTableElement));
      }
      
      // return root element
      return serviceStateTableElement;
   }
   
   /**
    * Converts this object from an XmlElement.
    *
    * @param element the XmlElement to convert from.
    * 
    * @return true if successful, false otherwise.
    */
   @Override
   public boolean convertFromXmlElement(XmlElement element)   
   {
      boolean rval = true;
      
      // clear table
      clear();
      
      // get all the state variables
      for(XmlElement variableElement: element.getChildren("stateVariable"))
      {
         UPnPServiceStateVariable variable = new UPnPServiceStateVariable();
         if(variable.convertFromXmlElement(variableElement))
         {
            // add variable
            addStateVariable(variable);
         }
      }
      
      return rval;
   }
   
   /**
    * Adds a state variable to this table.
    * 
    * @param variable the UPnPServiceStateVariable to add to this table. 
    */
   public void addStateVariable(UPnPServiceStateVariable variable)
   {
      mVariables.put(variable.getName(), variable);
   }
   
   /**
    * Removes a state variable from this table.
    * 
    * @param name the name of the state variable to remove.
    */
   public void removeStateVariable(String name)
   {
      mVariables.remove(name);
   }
   
   /**
    * Gets a state variable from this table by its name. This method will
    * return null if there is no state variable with the passed name in
    * this table.
    * 
    * @param name the name of the state variable to retrieve.
    * 
    * @return a state variable from this table by its name (can be null).
    */
   public UPnPServiceStateVariable getStateVariable(String name)
   {
      return mVariables.get(name);
   }
   
   /**
    * Gets all of the state variables from this table.
    * 
    * @return a collection of all of the UPnPServiceStateVariables from this
    *         table.
    */
   public Collection<UPnPServiceStateVariable> getStateVariables()
   {
      return mVariables.values();
   }
   
   /**
    * Clears all of the state variables from this table.
    */
   public void clear()
   {
      mVariables.clear();
   }
   
   /**
    * Gets an iterator over the state variables in this table.
    *
    * @return an iterator over the UPnPServiceStateVariables in this table.
    */
   public Iterator<UPnPServiceStateVariable> iterator()
   {
      return getStateVariables().iterator();
   }
   
   /**
    * Gets the logger for this UPnPServiceStateTable.
    * 
    * @return the logger for this UPnPServiceStateTable.
    */
   @Override
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
