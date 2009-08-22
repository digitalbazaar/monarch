/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;
import com.db.xml.XmlException;

/**
 * A UPnPServiceActionList is a XML serializable list of UPnPServiceActions.
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
 * actionList 
 * Required if and only if the service has actions. (Each service may have >= 0
 * actions.) Contains the following sub element(s): 
 * -----------------------------------------------------------------------
 * 
 * @author Dave Longley
 */
public class UPnPServiceActionList extends AbstractXmlSerializer
implements Iterable<UPnPServiceAction>
{
   /**
    * The UPnPServiceActions for this list.
    */
   protected List<UPnPServiceAction> mActions;
   
   /**
    * Creates a new UPnPServiceActionList.
    */
   public UPnPServiceActionList()
   {
      // create the actions list
      mActions = new LinkedList<UPnPServiceAction>();
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   @Override
   public String getRootTag()   
   {
      return "actionList";
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
      XmlElement listElement = new XmlElement(getRootTag());
      listElement.setParent(parent);
      
      // convert each action to an xml element child
      for(UPnPServiceAction action: this)
      {
         listElement.addChild(action.convertToXmlElement(listElement));
      }
      
      // return root element
      return listElement;      
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
      
      // clear action list
      clear();
      
      // convert actions
      for(XmlElement actionElement: element.getChildren("action"))
      {
         UPnPServiceAction action = new UPnPServiceAction();
         action.convertFromXmlElement(actionElement);
         addAction(action);
      }
   }
   
   /**
    * Adds a UPnPServiceAction to this list.
    * 
    * @param action the UPnPServiceAction to add.
    */
   public void addAction(UPnPServiceAction action)
   {
      getActions().add(action);
   }
   
   /**
    * Removes a UPnPServiceAction from this list.
    * 
    * @param action the UPnPServiceAction to remove.
    */
   public void removeAction(UPnPServiceAction action)
   {
      getActions().remove(action);
   }
   
   /**
    * Gets an action from this list by its name.
    * 
    * @param name the name of the action to retrieve.
    * 
    * @return the retrieved action or null if no action with the given name
    *         exists.
    */
   public UPnPServiceAction getAction(String name)
   {
      UPnPServiceAction rval = null;
      
      for(Iterator i = getActions().iterator(); i.hasNext() && rval == null;)
      {
         UPnPServiceAction action = (UPnPServiceAction)i.next();
         if(action.getName().equals(name))
         {
            rval = action;
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the UPnPServiceActions for this list in a vector.
    * 
    * @return the UPnPServiceActions for this list in a vector.
    */
   public List<UPnPServiceAction> getActions()
   {
      return mActions;
   }
   
   /**
    * Clears the actions from this list.
    */
   public void clear()
   {
      getActions().clear();
   }
   
   /**
    * Gets an iterator over the actions in this list.
    *
    * @return an iterator over the UPnPServiceActions in this list.
    */
   public Iterator<UPnPServiceAction> iterator()
   {
      return getActions().iterator();
   }
   
   /**
    * Gets the number of actions in this list.
    * 
    * @return the number of actions in this list.
    */
   public int getActionCount()
   {
      return getActions().size();
   }   
   
   /**
    * Gets the logger for this UPnPServiceActionList.
    * 
    * @return the logger for this UPnPServiceActionList.
    */
   @Override
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
