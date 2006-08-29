/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;

/**
 * An XmlAttributeMap is a map of XML attributes for an XML element. Each
 * attribute has a string for a name and some value that is either a
 * string or some other primitive type.
 * 
 * @author Dave Longley
 */
public class XmlAttributeMap
{
   /**
    * The underlying vector that stores the attribute keys and values.
    */
   protected Vector mAttributes;
   
   /**
    * A hashmap for quick attribute look up according to attribute name.
    */
   protected HashMap mAttributeMap;
   
   /**
    * A map that maps namespace prefixes to namespace URIs.
    */
   protected HashMap mNamespaceUriMap;
   
   /**
    * Creates a new empty XmlAttributeMap.
    */
   public XmlAttributeMap()
   {
      // create the underlying attributes vector
      mAttributes = new Vector();
      
      // create the look up map
      mAttributeMap = new HashMap();
      
      // create the namespace URI map
      mNamespaceUriMap = new HashMap();
   }
   
   /**
    * Adds an attribute to this map.
    * 
    * @param name the name of the attribute.
    * @param value the value of the attribute.
    */
   public void addAttribute(String name, String value)
   {
      addAttribute(name, value, null, null);
   }
   
   /**
    * Adds an attribute to this map.
    * 
    * @param name the name of the attribute.
    * @param value the value of the attribute.
    * @param namespace the namespace for the attribute.
    * @param namespaceUri the URI that points to the definition of the
    *                     namespace for the attribute.
    */
   public void addAttribute(
      String name, String value, String namespace, String namespaceUri)
   {
      if(namespace != null)
      {
         // use fully qualified name
         name = namespace + ":" + name;
         
         // add namespace -> namespace uri mapping
         mNamespaceUriMap.put(namespace, namespaceUri);
      }
      
      // add attribute and mapping
      XmlAttribute attribute = new XmlAttribute(name, value);
      mAttributes.add(attribute);
      mAttributeMap.put(name, attribute);
   }
   
   /**
    * Adds an attribute to this map.
    * 
    * @param name the name of the attribute.
    * @param value the value of the attribute.
    */
   public void addAttribute(String name, int value)
   {
      addAttribute(name, "" + value);
   }
   
   /**
    * Adds an attribute to this map.
    * 
    * @param name the name of the attribute.
    * @param value the value of the attribute.
    */
   public void addAttribute(String name, long value)
   {
      addAttribute(name, "" + value);
   }
   
   /**
    * Adds an attribute to this map.
    * 
    * @param name the name of the attribute.
    * @param value the value of the attribute.
    */
   public void addAttribute(String name, float value)
   {
      addAttribute(name, "" + value);
   }
   
   /**
    * Adds an attribute to this map.
    * 
    * @param name the name of the attribute.
    * @param value the value of the attribute.
    */
   public void addAttribute(String name, double value)
   {
      addAttribute(name, "" + value);
   }
   
   /**
    * Adds an attribute to this map.
    * 
    * @param name the name of the attribute.
    * @param value the value of the attribute.
    */
   public void addAttribute(String name, boolean value)
   {
      addAttribute(name, "" + value);
   }
   
   /**
    * Removes an attribute from this map.
    * 
    * @param name the name of the attribute to remove from this map.
    */
   public void removeAttribute(String name)
   {
      // get the namespace for the name
      String namespace = XmlElement.getNamespacePrefix(name);
      if(namespace != null)
      {
         // see if the attribute namespace uri should be removed
         int count = 0;
         for(Iterator i = mAttributeMap.keySet().iterator();
             i.hasNext() && count < 2;)
         {
            String attributeName = (String)i.next();
            
            // get the namespace for the attribute name
            String ns = XmlElement.getNamespacePrefix(attributeName);
            if(ns != null && ns.equals(namespace))
            {
               count++;
            }
         }
         
         if(count < 2)
         {
            // remove the namespace uri mapping
            mNamespaceUriMap.remove(namespace);
         }
      }
      
      // remove the attribute
      XmlAttribute attribute = (XmlAttribute)mAttributeMap.get(name);
      if(attribute != null)
      {
         mAttributes.remove(attribute);
      }
      
      // remove the attribute mapping
      mAttributeMap.remove(name);
   }
   
   /**
    * Clears all attributes from this map.
    */
   public void clear()
   {
      mAttributes.clear();
      mAttributeMap.clear();
      mNamespaceUriMap.clear();
   }
   
   /**
    * Gets the namespace for the given namespace URI. This may return null
    * if there is no namespace that matches the given namespace URI.
    * 
    * @param namespaceUri the namespace URI to get the namespace prefix for.
    * 
    * @return the namespace for the namespace URI or null if none is found.
    */
   public String getNamespace(String namespaceUri)
   {
      String rval = null;
      
      if(namespaceUri != null)
      {
         // iterate through the namespace URI map
         for(Iterator i = mNamespaceUriMap.keySet().iterator();
             i.hasNext() && rval == null;)
         {
            String namespace = (String)i.next();
            String nsUri = (String)mNamespaceUriMap.get(namespace);
            if(namespaceUri.equals(nsUri))
            {
               rval = namespace;
            }
         }
      }
      
      return rval;
   }      
   
   /**
    * Gets the namespace URI for the attribute with the given name. This may
    * return null if there is no namespace URI for the given attribute or if
    * the attribute is not in this map.
    * 
    * @param name the name of the attribute to get the namespace URI for.
    * 
    * @return the namespace URI for the attribute or null if none is found.
    */
   public String getNamespaceUri(String name)
   {
      String rval = null;

      if(hasAttribute(name))
      {
         // get the namespace for the name
         String namespace = XmlElement.getNamespacePrefix(name);
         rval = (String)mNamespaceUriMap.get(namespace);
      }
      
      return rval;
   }
   
   /**
    * Gets an attribute value from this map according to its name as a string
    * representation, regardless of its actual type.
    * 
    * @param name the name of the attribute.
    * 
    * @return the value of the attribute as a string or a blank string if
    *         the attribute cannot be found.
    */
   public String getAttributeValue(String name)
   {
      String rval = "";
      
      if(hasAttribute(name))
      {
         XmlAttribute attribute = (XmlAttribute)mAttributeMap.get(name);
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
      return getAttributeValue(name);
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
      int rval = 0;
      
      try
      {
         rval = Integer.parseInt(getAttributeValue(name));
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
      long rval = 0;
      
      try
      {
         rval = Long.parseLong(getAttributeValue(name));
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
      float rval = 0;
      
      try
      {
         rval = Float.parseFloat(getAttributeValue(name));
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
      double rval = 0;
      
      try
      {
         rval = Double.parseDouble(getAttributeValue(name));
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
      boolean rval = false;
      
      try
      {
         rval = Boolean.parseBoolean(getAttributeValue(name));
      }
      catch(Throwable ignore)
      {
      }
      
      return rval;
   }
   
   /**
    * Gets whether or not an attribute with the specified name exists in
    * this map.
    * 
    * @param name the name of the attribute to look for.
    * 
    * @return true if the attribute exists in this map, false if not.
    */
   public boolean hasAttribute(String name)
   {
      return mAttributeMap.containsKey(name);
   }
   
   /**
    * Gets a collection of the attribute names in this map. Changes to the
    * passed collection will not be reflected in this map.
    * 
    * @return a set of the attribute names in this map.
    */
   public Collection getAttributeNames()
   {
      Vector names = new Vector();
      for(Iterator i = mAttributes.iterator(); i.hasNext();)
      {
         XmlAttribute attribute = (XmlAttribute)i.next();
         names.add(attribute.getName());
      }
      
      return names;
   }
   
   /**
    * Gets a collection of the attribute values in this map. Changes to the
    * passed collection will not be reflected in this map.
    * 
    * @return a set of the attribute values in this map.
    */
   public Collection getAttributeValues()
   {
      Vector values = new Vector();
      for(Iterator i = mAttributes.iterator(); i.hasNext();)
      {
         XmlAttribute attribute = (XmlAttribute)i.next();
         values.add(attribute.getValue());
      }
      
      return values;
   }
   
   /**
    * Gets the number of attributes in this map.
    * 
    * @return the number of attributes in this map.
    */
   public int getAttributeCount()
   {
      return mAttributes.size();
   }
}
