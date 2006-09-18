/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;

/**
 * An XmlAttributeMap is a list of XML attributes for an XML element. This
 * list contains XmlAttribute objects.
 * 
 * @author Dave Longley
 */
public class XmlAttributeList
{
   /**
    * The XmlElement this list is for.
    */
   protected XmlElement mXmlElement;
   
   /**
    * The underlying vector that stores the XmlAttributes.
    */
   protected Vector mAttributes;
   
   /**
    * A map that maps namespace prefixes to namespace URIs.
    */
   protected HashMap mNamespaceUriMap;
   
   /**
    * Creates a new empty XmlAttributeList for the specified XmlElement.
    * 
    * @param element the XmlElement this list is for.
    */
   public XmlAttributeList(XmlElement element)
   {
      mXmlElement = element;
      
      // create the underlying attributes vector
      mAttributes = new Vector();
      
      // create the namespace URI map
      mNamespaceUriMap = new HashMap();
   }
   
   /**
    * Adds a namespace mapping to this list.
    * 
    * @param namespacePrefix the namespace prefix.
    * @param namespaceUri the namespace URI.
    */
   public void addNamespaceMapping(String namespacePrefix, String namespaceUri)
   {
      mNamespaceUriMap.put(namespacePrefix, namespaceUri);
   }
   
   /**
    * Gets the XmlElement this list is for.
    * 
    * @return the XmlElement this list is for.
    */
   public XmlElement getXmlElement()
   {
      return mXmlElement;
   }
   
   /**
    * Adds an attribute to this list using the list's XmlElement's namespace
    * URI.
    * 
    * @param name the local name of the attribute, unless the attribute is
    *             defining a namespace with "xmlns" in which case the
    *             qualified name should be passed.
    * @param value the value of the attribute.
    */
   public void addAttribute(String name, String value)
   {
      if(name.equals("xmlns") || name.startsWith("xmlns:"))
      {
         addAttribute(name, value, null);
      }
      else
      {
         addAttribute(name, value, getXmlElement().getNamespaceUri());
      }
   }
   
   /**
    * Adds an attribute to this list.
    * 
    * @param name the local name of the attribute, unless the attribute is
    *             defining a namespace prefix -> namespace URI mapping
    *             using "xmlns".
    * @param value the value of the attribute.
    * @param namespaceUri the URI that points to the definition of the
    *                     namespace for the attribute.
    */
   public void addAttribute(
      String name, String value, String namespaceUri)
   {
      addAttribute(name, value, namespaceUri, true);
   }
   
   /**
    * Adds an attribute to this list.
    * 
    * @param name the local name of the attribute, unless the attribute is
    *             defining a namespace prefix -> namespace URI mapping
    *             using "xmlns".
    * @param value the value of the attribute.
    * @param namespaceUri the URI that points to the definition of the
    *                     namespace for the attribute.
    * @param inherit true to inherit this XmlElement's namespace URI if
    *                the namespace URI is set to null (and not defining
    *                a namespace), false not to.
    */
   public void addAttribute(
      String name, String value, String namespaceUri, boolean inherit)
   {
      // only add the attribute if it isn't already in this map
      if(!hasAttribute(name, namespaceUri))
      {
         // add namespace mapping, if appropriate
         if(name.equals("xmlns"))
         {
            // defining the default namespace
            addNamespaceMapping(null, value);
         }
         else if(name.startsWith("xmlns:"))
         {
            // get the local name for the attribute
            String localName = XmlElement.parseLocalName(name);
            
            // defining a namespace with a prefix that is the local name
            addNamespaceMapping(localName, value);
         }
         
         // add attribute
         XmlAttribute attribute = new XmlAttribute(name, value, namespaceUri);
         attribute.setParent(getXmlElement());
         mAttributes.add(attribute);
      }
   }
   
   /**
    * Adds an attribute to this list using the list's XmlElement's
    * namespace URI.
    * 
    * @param name the local name of the attribute.
    * @param value the value of the attribute.
    */
   public void addAttribute(String name, int value)   
   {
      addAttribute(name, value, getXmlElement().getNamespaceUri());
   }
   
   /**
    * Adds an attribute to this list.
    * 
    * @param name the local name of the attribute.
    * @param value the value of the attribute.
    * @param namespaceUri the URI that points to the definition of the
    *                     namespace for the attribute.
    */
   public void addAttribute(String name, int value, String namespaceUri)
   {
      addAttribute(name, "" + value, namespaceUri);
   }
   
   /**
    * Adds an attribute to this list using this list's XmlElement's
    * namespace URI.
    * 
    * @param name the local name of the attribute.
    * @param value the value of the attribute.
    */
   public void addAttribute(String name, long value)
   {
      addAttribute(name, value, getXmlElement().getNamespaceUri());
   }
   
   /**
    * Adds an attribute to this list.
    * 
    * @param name the local name of the attribute.
    * @param value the value of the attribute.
    * @param namespaceUri the URI that points to the definition of the
    *                     namespace for the attribute.
    */
   public void addAttribute(String name, long value, String namespaceUri)
   {
      addAttribute(name, "" + value, namespaceUri);
   }
   
   /**
    * Adds an attribute to this list using this list's XmlElement's
    * namespace URI.
    * 
    * @param name the local name of the attribute.
    * @param value the value of the attribute.
    */
   public void addAttribute(String name, float value)   
   {
      addAttribute(name, value, getXmlElement().getNamespaceUri());
   }
   
   /**
    * Adds an attribute to this list.
    * 
    * @param name the local name of the attribute.
    * @param value the value of the attribute.
    * @param namespaceUri the URI that points to the definition of the
    *                     namespace for the attribute.
    */
   public void addAttribute(String name, float value, String namespaceUri)
   {
      addAttribute(name, "" + value, namespaceUri);
   }

   /**
    * Adds an attribute to this list using this list's XmlElement's
    * namespace URI.
    * 
    * @param name the local name of the attribute.
    * @param value the value of the attribute.
    */
   public void addAttribute(String name, double value)
   {
      addAttribute(name, value, getXmlElement().getNamespaceUri());
   }
   
   /**
    * Adds an attribute to this list.
    * 
    * @param name the local name of the attribute.
    * @param value the value of the attribute.
    * @param namespaceUri the URI that points to the definition of the
    *                     namespace for the attribute.
    */
   public void addAttribute(String name, double value, String namespaceUri)
   {
      addAttribute(name, "" + value, namespaceUri);
   }
   
   /**
    * Adds an attribute to this list using this list's XmlElement's
    * namespace URI.
    * 
    * @param name the local name of the attribute.
    * @param value the value of the attribute.
    */
   public void addAttribute(String name, boolean value)
   {
      addAttribute(name, value, getXmlElement().getNamespaceUri());
   }
   
   /**
    * Adds an attribute to this list.
    * 
    * @param name the local name of the attribute.
    * @param value the value of the attribute.
    * @param namespaceUri the URI that points to the definition of the
    *                     namespace for the attribute.
    */
   public void addAttribute(String name, boolean value, String namespaceUri)
   {
      addAttribute(name, "" + value, namespaceUri);
   }
   
   /**
    * Removes an attribute from this list using this list's XmlElement's
    * namespace URI.
    * 
    * @param name the local name of the attribute to remove from this list.
    */
   public void removeAttribute(String name)
   {
      removeAttribute(name, getXmlElement().getNamespaceUri());
   }
   
   /**
    * Removes an attribute from this list.
    * 
    * @param name the local name of the attribute to remove from this list.
    * @param namespaceUri the URI that points to the definition of the
    *                     namespace for the attribute.
    */
   public void removeAttribute(String name, String namespaceUri)
   {
      // get the attribute
      XmlAttribute attribute = getAttribute(name, namespaceUri);
      if(attribute != null)
      {
         // remove the attribute
         getAttributes().remove(attribute);
      }
   }
   
   /**
    * Clears all attributes and namespace mappings from this list.
    */
   public void clear()
   {
      getAttributes().clear();
      mNamespaceUriMap.clear();
   }
   
   /**
    * Gets whether or not an attribute with the specified name exists in
    * this map.
    * 
    * @param name the local name of the attribute to look for unless the
    *             attribute is used to define a namespace with "xmlns" in
    *             which case the qualified name should be passed.
    * @param namespaceUri the namespace URI for the attribute.
    * 
    * @return true if the attribute exists in this map, false if not.
    */
   public boolean hasAttribute(String name, String namespaceUri)
   {
      return getAttribute(name, namespaceUri) != null;
   }
   
   /**
    * Gets and XmlAttribute by its name and namespace URI.
    * 
    * @param name the local name of the attribute to get unless the
    *             attribute is used to define a namespace with "xmlns" in
    *             which case the qualified name should be passed.
    * @param namespaceUri the namespace URI for the attribute to get.
    * 
    * @return the XmlAttribute or null if a match was not found. 
    */
   public XmlAttribute getAttribute(String name, String namespaceUri)
   {
      XmlAttribute rval = null;
      
      // if the attribute name is "xmlns" or "xmlns:localname" then
      // the attribute name is reserved and the namespace URI must be null
      if(name.equals("xmlns") || name.startsWith("xmlns:"))
      {
         if(namespaceUri != null)
         {
            throw new IllegalArgumentException(
               "An XML attribute with the name 'xmlns', 'xmlns:localname' " +
               "is reserved and must have a null namespace URI.");
         }
      }
      
      for(Iterator i = getAttributes().iterator(); i.hasNext() && rval == null;)
      {
         XmlAttribute attribute = (XmlAttribute)i.next();
         String attributeNamespaceUri = attribute.getNamespaceUri();
         
         // check attribute names
         if(attribute.getName().equals(name))
         {
            // check attribute namespaces
            if((attributeNamespaceUri == null && namespaceUri == null) ||
               (attributeNamespaceUri != null &&
                attributeNamespaceUri.equals(namespaceUri)))
            {
               rval = attribute;
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Gets a collection of the attributes in this map. Changes to the
    * passed collection will be reflected in this map.
    * 
    * @return a set of the attributes in this map.
    */
   public Collection getAttributes()
   {
      return mAttributes;
   }
   
   /**
    * Gets the number of attributes in this map.
    * 
    * @return the number of attributes in this map.
    */
   public int getAttributeCount()
   {
      return getAttributes().size();
   }
   
   /**
    * Gets whether or not a namespace prefix for the given namespace URI
    * is defined in this scope.
    * 
    * @param namespaceUri the namespace URI to check for.
    * 
    * @return true if there is a namespace prefix defined at this scope for
    *         the given namespace URI, false if not.
    */
   public boolean isNamespacePrefixDefined(String namespaceUri)
   {
      boolean rval = false;
      
      if(namespaceUri != null)
      {
         // iterate through the namespace URI map
         for(Iterator i = mNamespaceUriMap.keySet().iterator();
             i.hasNext() && !rval;)
         {
            String namespacePrefix = (String)i.next();
            String uri = (String)mNamespaceUriMap.get(namespacePrefix);
            if(namespaceUri.equals(uri))
            {
               rval = true;
            }
         }
      }
      
      return rval;      
   }
   
   /**
    * Finds the namespace prefix for the given namespace URI. This may return
    * null if there is no namespace prefix that matches the given namespace
    * URI or if the passed namespace URI points to the default namespace.
    * 
    * @param namespaceUri the namespace URI to get the namespace prefix for.
    * 
    * @return the namespace prefix for the namespace URI or null if none is
    *         found.
    */
   public String findNamespacePrefix(String namespaceUri)
   {
      String rval = null;
      
      if(namespaceUri != null)
      {
         // iterate through the namespace URI map
         for(Iterator i = mNamespaceUriMap.keySet().iterator();
             i.hasNext() && rval == null;)
         {
            String namespacePrefix = (String)i.next();
            String uri = (String)mNamespaceUriMap.get(namespacePrefix);
            if(namespaceUri.equals(uri))
            {
               rval = namespacePrefix;
            }
         }
      }
      
      return rval;
   }  
   
   /**
    * Gets whether or not a namespace URI for the given namespace prefix
    * is defined in this scope.
    * 
    * @param namespacePrefix the namespace prefix to check for.
    * 
    * @return true if there is a namespace URI defined at this scope for
    *         the given namespace prefix, false if not.
    */
   public boolean isNamespaceUriDefined(String namespacePrefix)
   {
      return mNamespaceUriMap.containsKey(namespacePrefix);
   }
   
   /**
    * Finds the namespace URI for the given namespace prefix. This will return
    * null if there is no namespace URI that matches the given namespace
    * prefix.
    * 
    * @param namespacePrefix the namespace prefix to get the namespace prefix
    *                        for.
    * 
    * @return the namespace URI for the namespace prefix or null if none is
    *         found.
    */
   public String findNamespaceUri(String namespacePrefix)
   {
      return (String)mNamespaceUriMap.get(namespacePrefix);
   }
   
   /**
    * Gets an attribute value from this map according to its name. The value is
    * returned as a string regardless of its actual type.
    * 
    * @param name the name of the attribute.
    * 
    * @return the value of the attribute as a string or a blank string if
    *         the attribute cannot be found.
    */
   public String getAttributeValue(String name)   
   {
      return getAttributeValue(name, getXmlElement().getNamespaceUri());
   }
   
   /**
    * Gets an attribute value from this map according to its name and
    * namespace URI. The value is returned as a string regardless of its
    * actual type.
    * 
    * @param name the name of the attribute.
    * @param namespaceUri the namespace URI of the attribute. 
    * 
    * @return the value of the attribute as a string or a blank string if
    *         the attribute cannot be found.
    */
   public String getAttributeValue(String name, String namespaceUri)
   {
      String rval = "";
      
      XmlAttribute attribute = getAttribute(name, namespaceUri);
      if(attribute != null)
      {
         rval = attribute.getValue();
      }
      
      return rval;
   }
   
   /**
    * Gets an attribute string value from this map according to its name.
    * 
    * @param name the name of the attribute.
    * 
    * @return the value of the attribute or a blank string if the attribute
    *         cannot be found.
    */
   public String getAttributeStringValue(String name)
   {
      return getAttributeStringValue(name, getXmlElement().getNamespaceUri());
   }   
   
   /**
    * Gets an attribute string value from this map according to its name
    * and namespace URI.
    * 
    * @param name the name of the attribute.
    * @param namespaceUri the namespace URI of the attribute.
    * 
    * @return the value of the attribute or a blank string if the attribute
    *         cannot be found.
    */
   public String getAttributeStringValue(String name, String namespaceUri)
   {
      return getAttributeValue(name, namespaceUri);
   }
   
   /**
    * Gets an attribute int value from this map according to its name.
    * 
    * @param name the name of the attribute.
    * 
    * @return the value of the attribute or 0 if the attribute cannot
    *         be found.
    */
   public int getAttributeIntValue(String name)   
   {
      return getAttributeIntValue(name, getXmlElement().getNamespaceUri());
   }
   
   /**
    * Gets an attribute int value from this map according to its name
    * and namespace URI.
    * 
    * @param name the name of the attribute.
    * @param namespaceUri the namespace URI of the attribute.
    * 
    * @return the value of the attribute or 0 if the attribute cannot
    *         be found.
    */
   public int getAttributeIntValue(String name, String namespaceUri)
   {
      int rval = 0;
      
      try
      {
         rval = Integer.parseInt(getAttributeValue(name, namespaceUri));
      }
      catch(Throwable ignore)
      {
      }
      
      return rval;
   }
   
   /**
    * Gets an attribute long value from this map according to its name.
    * 
    * @param name the name of the attribute.
    * 
    * @return the value of the attribute or 0 if the attribute cannot
    *         be found.
    */
   public long getAttributeLongValue(String name)   
   {
      return getAttributeLongValue(name, getXmlElement().getNamespaceUri());
   }
   
   /**
    * Gets an attribute long value from this map according to its name
    * and namespace URI.
    * 
    * @param name the name of the attribute.
    * @param namespaceUri the namespace URI of the attribute.
    * 
    * @return the value of the attribute or 0 if the attribute cannot
    *         be found.
    */
   public long getAttributeLongValue(String name, String namespaceUri)
   {
      long rval = 0;
      
      try
      {
         rval = Long.parseLong(getAttributeValue(name, namespaceUri));
      }
      catch(Throwable ignore)
      {
      }
      
      return rval;
   }
   
   /**
    * Gets an attribute float value from this map according to its name.
    * 
    * @param name the name of the attribute.
    * 
    * @return the value of the attribute or 0 if the attribute cannot
    *         be found.
    */
   public float getAttributeFloatValue(String name)   
   {
      return getAttributeFloatValue(name, getXmlElement().getNamespaceUri());
   }
   
   /**
    * Gets an attribute float value from this map according to its name
    * and namespace URI.
    * 
    * @param name the name of the attribute.
    * @param namespaceUri the namespace URI of the attribute.
    * 
    * @return the value of the attribute or 0 if the attribute cannot
    *         be found.
    */
   public float getAttributeFloatValue(String name, String namespaceUri)
   {
      float rval = 0;
      
      try
      {
         rval = Float.parseFloat(getAttributeValue(name, namespaceUri));
      }
      catch(Throwable ignore)
      {
      }      
      
      return rval;
   }
   
   /**
    * Gets an attribute double value from this map according to its name.
    * 
    * @param name the name of the attribute.
    * 
    * @return the value of the attribute or 0 if the attribute cannot
    *         be found.
    */
   public double getAttributeDoubleValue(String name)   
   {
      return getAttributeDoubleValue(name, getXmlElement().getNamespaceUri());
   }
   
   /**
    * Gets an attribute double value from this map according to its name
    * and namespace URI.
    * 
    * @param name the name of the attribute.
    * @param namespaceUri the namespace URI of the attribute.
    * 
    * @return the value of the attribute or 0 if the attribute cannot
    *         be found.
    */
   public double getAttributeDoubleValue(String name, String namespaceUri)
   {
      double rval = 0;
      
      try
      {
         rval = Double.parseDouble(getAttributeValue(name, namespaceUri));
      }
      catch(Throwable ignore)
      {
      }
      
      return rval;
   }
   
   /**
    * Gets an attribute boolean value from this map according to its name.
    * 
    * @param name the name of the attribute.
    * 
    * @return the value of the attribute or false if the attribute cannot
    *         be found.
    */
   public boolean getAttributeBooleanValue(String name)   
   {
      return getAttributeBooleanValue(name, getXmlElement().getNamespaceUri());
   }
   
   /**
    * Gets an attribute boolean value from this map according to its name
    * and namespace URI.
    * 
    * @param name the name of the attribute.
    * @param namespaceUri the namespace URI of the attribute.
    * 
    * @return the value of the attribute or false if the attribute cannot
    *         be found.
    */
   public boolean getAttributeBooleanValue(String name, String namespaceUri)
   {
      boolean rval = false;
      
      try
      {
         rval = Boolean.parseBoolean(getAttributeValue(name, namespaceUri));
      }
      catch(Throwable ignore)
      {
      }
      
      return rval;
   }
}
