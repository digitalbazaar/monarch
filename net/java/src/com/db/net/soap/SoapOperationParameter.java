/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.IXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A SoapOperationParameter is a parameter for a SoapOperation that is
 * performed over SOAP (Simple Object Access Protocol). It has a target
 * namespace URI (a location that defines the XML elements of the parameter),
 * a name, and a value.
 * 
 * This parameter can be converted to and from XML for SOAP transportation.
 * 
 * FUTURE CODE: When we move over to SOAP 1.2 or (even before that if we
 * add complex object support) we want a clean redesign of the soap classes
 * and interfaces to make it more streamlined and easy to use. This may
 * include created a new interface for serializing complex objects to xml.  
 * 
 * @author Dave Longley
 */
public class SoapOperationParameter extends AbstractXmlSerializer
{
   /**
    * The name of this parameter. This is used if this parameter is a primitive
    * type.
    */
   protected String mName;
   
   /**
    * The value for this parameter. This is used if this parameter is a
    * primitive type.
    */
   protected String mValue;

   /**
    * The target namespace URI that defines the XML elements of this parameter.
    */
   protected String mTargetNamespaceUri;
   
   /**
    * The XML serializer to use to convert this parameter to XML. This
    * may be null in the case that the parameter is a primitive type.
    */
   protected IXmlSerializer mSerializer;
   
   /**
    * Creates a new empty SoapOperationParameter. An XmlElement will be
    * used to convert this parameter from XML.
    */
   public SoapOperationParameter()
   {
      this(new XmlElement());
   }
   
   /**
    * Creates a new SoapOperationParameter.
    * 
    * @param name the name of the parameter.
    * @param value the value of the parameter.
    * @param tns the URI that defines the XML elements of this parameter.
    */
   public SoapOperationParameter(String name, String value, String tns)
   {
      // store name, value, and target namespace URI
      setName(name);
      setValue(value);
      mTargetNamespaceUri = tns;
   }
   
   /**
    * Creates a new SoapOperationParameter with the specified IXmlSerializer
    * that will be used to convert to and from XML. The target namespace
    * will be read when converting from XML.
    * 
    * @param serializer the IXmlSerializer used to convert to and from XML.
    */
   public SoapOperationParameter(IXmlSerializer serializer)   
   {
      this(serializer, null);
   }
   
   /**
    * Creates a new SoapOperationParameter with the specified IXmlSerializer
    * that will be used to convert to and from XML.
    * 
    * @param serializer the IXmlSerializer used to convert to and from XML.
    * @param tns the URI that defines the XML elements of this parameter.
    */
   public SoapOperationParameter(IXmlSerializer serializer, String tns)
   {
      // store the serializer and the target namespace URI
      mSerializer = serializer;
      mTargetNamespaceUri = tns;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   @Override
   public String getRootTag()   
   {
      String tag = "";
      
      if(mSerializer != null)
      {
         tag = mSerializer.getRootTag();
      }
      else
      {
         tag = getName();
      }
      
      return tag;
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
      XmlElement element = null;
      
      // use the xml serializer if it is available
      if(mSerializer != null)
      {
         element = mSerializer.convertToXmlElement(parent);
      }
      else
      {
         // create element, use parameter name as the tag name
         element = new XmlElement(getRootTag());
         element.setParent(parent);
         
         // set value to parameter value
         element.setValue(getValue());
      }
      
      // do not inherit parent namespace
      element.setInheritNamespaceUri(false);
      
      if(mTargetNamespaceUri != null)
      {
         // set the namespace and add the attribute defining the namespace
         element.setNamespaceUri(mTargetNamespaceUri);
         element.addAttribute("xmlns:tns", mTargetNamespaceUri);
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
   @Override
   public boolean convertFromXmlElement(XmlElement element)   
   {
      boolean rval = true;
      
      // use the xml serializer if it is available
      if(mSerializer != null)
      {
         rval = mSerializer.convertFromXmlElement(element);
         
         // see if the xml element has children
         if(!element.hasChildren())
         {
            // parse out the name and value
            mName = element.getName();
            mValue = element.getValue();
         }
      }
      else
      {
         // get the name of the parameter as the element name
         setName(element.getName());
         
         // get the value of the parameter as the element value
         setValue(element.getValue());
      }
      
      // get the target namespace uri
      mTargetNamespaceUri = element.getNamespaceUri();
      
      return rval;
   }
   
   /**
    * Sets the name for this parameter.
    * 
    * @param name the name for this parameter.
    */
   public void setName(String name)
   {
      mName = name;
   }
   
   /**
    * Gets the name for this parameter.
    * 
    * @return the name for this parameter.
    */
   public String getName()
   {
      String rval = "";
      
      if(isPrimitive())
      {
         rval = mName;
      }
      
      return rval;
   }
   
   /**
    * Sets the target namespace URI that defines the XML elements of this
    * parameter.
    * 
    * @param tns the target namespace URI that defines the XML elements of this
    *            parameter (can be null).
    */
   public void setTargetNamespaceUri(String tns)
   {
      mTargetNamespaceUri = tns;
   }
   
   /**
    * Gets the target namespace URI that defines the XML elements of this
    * parameter.
    * 
    * @return the target namespace URI that defines the XML elements of this
    *         parameter (can be null).
    */
   public String getTargetNamespaceUri()
   {
      return mTargetNamespaceUri;
   }
   
   /**
    * Sets the value for this parameter.
    * 
    * @param value the value for this parameter.
    */
   public void setValue(String value)
   {
      mValue = value;
   }
   
   /**
    * Gets the value for this parameter.
    * 
    * @return the value for this parameter.
    */
   public String getValue()
   {
      String rval = "";
      
      if(isPrimitive())
      {
         rval = mValue;
      }
      
      return rval;
   }
   
   /**
    * Gets the value of this parameter as an XmlElement if this parameter is
    * not a primitive type.
    * 
    * @return the value of this parameter as an XmlElement if this parameter
    *         is not a primitive type, otherwise null is returned.
    */
   public XmlElement getValueAsXmlElement()
   {
      XmlElement rval = null;
      
      if(!isPrimitive() && mSerializer != null)
      {
         if(mSerializer instanceof XmlElement)
         {
            rval = (XmlElement)mSerializer;
         }
         else
         {
            rval = mSerializer.convertToXmlElement(null);
         }
      }
      
      return rval;
   }
   
   /**
    * Returns true if value of the parameter can be parsed as a primitive,
    * (vs. a complex object) false if not.
    * 
    * @return true if the value of this parameter can be parsed as a
    *         primitive (vs. a complex object), false if not.
    */
   public boolean isPrimitive()
   {
      boolean rval = false;
      
      if(mSerializer == null)
      {
         rval = true;
      }
      else if(mName != null)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets the logger for this SoapOperationParameter.
    * 
    * @return the logger for this SoapOperationParameter.
    */
   @Override
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
