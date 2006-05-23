/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import com.db.logging.LoggerManager;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;

import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * A convenience class that wraps a dom element.
 * 
 * @author Dave Longley
 */
public class ElementReader
{
   /**
    * The wrapped DOM element.
    */
   protected Element mElement;
   
   /**
    * A namespace URI to prefix map for this element (not including its
    * children).
    */
   protected HashMap mNamespaceToPrefixMap;
   
   /**
    * A prefix map to namespace URI for this element (not including its
    * children).
    */
   protected HashMap mPrefixToNamespaceMap;
   
   /**
    * Creates a new element reader wrapper around the passed element.
    * 
    * @param e the DOM element to wrap.
    */
   public ElementReader(Element e)
   {
      // e cannot be null
      if(e == null)
      {
         throw new IllegalArgumentException("Element cannot be null");
      }

      mElement = e;
      mNamespaceToPrefixMap = getNamespaceMap(false);
      mPrefixToNamespaceMap = getPrefixMap(false);
   }
   
   /**
    * Adds namespace URI to prefix mapping for this element to the passed map.
    * Will overwrite existing entries.
    * 
    * @param map a namespace URI to prefix map to update.
    * @param recursive true to recurse through children to add mappings,
    *                  false not to.
    */
   public void updateNamespaceMap(HashMap map, boolean recursive)
   {
      NamedNodeMap nnm = mElement.getAttributes();
      for(int i = 0; i < nnm.getLength(); i++)
      {
         Node node = nnm.item(i);
         String[] name = node.getNodeName().split(":");
         if(name.length > 1)
         {
            // see if this is a definition
            if(name[0].equals("xmlns"))
            {
               // add a map entry
               map.put(node.getNodeValue(), name[1]);
            }
         }
      }
      
      if(recursive)
      {
         // update map with child values
         Iterator ei = getElementReaders().iterator();
         while(ei.hasNext())
         {
            ElementReader er = (ElementReader)ei.next();
            er.updateNamespaceMap(map, true);
         }
      }
   }

   /**
    * Gets a namespace URI to prefix map for this element.
    * 
    * @param recursive true to recurse through children to add mappings,
    *                  false not to.
    * @return a namespace URI to prefix map.
    */
   public HashMap getNamespaceMap(boolean recursive)
   {
      HashMap map = new HashMap();
         
      // update the map with this element's values
      updateNamespaceMap(map, recursive);
      
      return map;
   }
   
   /**
    * Gets the namespace URI for a given prefix.
    * 
    * @param prefix the prefix to get the namespaceURI for.
    * @param recursive true to do a recursive look up through the element's
    *                  children, false not to.
    * @return the namespaceURI for the given prefix, null if not found.
    */
   public String getNamespaceURI(String prefix, boolean recursive)
   {
      HashMap map = (recursive) ? getPrefixMap(true) : mPrefixToNamespaceMap;
      
      return (String)map.get(prefix);
   }   
   
   /**
    * Adds prefix to namespace URI mapping for this element to the passed map.
    * Will overwrite existing entries.
    * 
    * @param map a prefix to namespace URI map to update.
    * @param recursive true to recurse through children to add mappings,
    *                  false not to.
    */
   public void updatePrefixMap(HashMap map, boolean recursive)
   {
      NamedNodeMap nnm = mElement.getAttributes();
      for(int i = 0; i < nnm.getLength(); i++)
      {
         Node node = nnm.item(i);
         String[] name = node.getNodeName().split(":");
         if(name.length > 1)
         {
            // see if this is a definition
            if(name[0].equals("xmlns"))
            {
               // add a map entry
               map.put(name[1], node.getNodeValue());
            }
         }
      }
      
      if(recursive)
      {
         // update map with child values
         Iterator ei = getElementReaders().iterator();
         while(ei.hasNext())
         {
            ElementReader er = (ElementReader)ei.next();
            er.updateNamespaceMap(map, true);
         }
      }
   }

   /**
    * Gets a prefix to namespace URI map for this element.
    * 
    * @param recursive true to recurse through children to add mappings,
    *                  false not to.
    * @return a prefix to namespace URI map.
    */
   public HashMap getPrefixMap(boolean recursive)
   {
      HashMap map = new HashMap();
         
      // update the map with this element's values
      updatePrefixMap(map, recursive);
      
      return map;
   }   
   
   /**
    * Gets the prefix for a given namespace URI.
    * 
    * @param namespaceURI the namespace URI to get the prefix for.
    * @param recursive true to do a recursive look up through the element's
    *                  children, false not to.
    * @return the prefix for the given namespace URI, null if not found.
    */
   public String getPrefix(String namespaceURI, boolean recursive)
   {
      HashMap map = (recursive) ? getNamespaceMap(true) : mNamespaceToPrefixMap;
      
      return (String)map.get(namespaceURI);
   }
   
   /**
    * Gets a vector of all the child elements.
    * 
    * @return a vector of elements.
    */
   public Vector getElements()
   {
      Vector elements = new Vector();
      
      NodeList list = mElement.getChildNodes();
      
      for(int i = 0; i < list.getLength(); i++)
      {
         if(list.item(i) instanceof Element)
         {
            elements.add(list.item(i));
         }
      }
      
      return elements;
   }
   
   /**
    * Gets a vector of all the child elements wrapped by element readers.
    * 
    * @return a vector of element readers.
    */
   public Vector getElementReaders()
   {
      Vector ereaders = new Vector();
      
      NodeList list = mElement.getChildNodes();
      
      for(int i = 0; i < list.getLength(); i++)
      {
         if(list.item(i) instanceof Element)
         {
            ereaders.add(new ElementReader((Element)list.item(i)));
         }
      }
      
      return ereaders;
   }

   /**
    * Gets a vector of elements that are children to this element
    * that match the passed tag name.
    * 
    * @param tagName the tag name of the elements.
    * @return a vector of elements.
    */
   public Vector getElements(String tagName)
   {
      Vector elements = new Vector();
      
      NodeList list = mElement.getChildNodes();
      for(int i = 0; i < list.getLength(); i++)
      {
         Node node = list.item(i);
         
         if(node.getNodeName().equals(tagName))
         {
            elements.add(node);
         }
      }
      
      return elements;
   }
   
   /**
    * Gets a vector of elements that are children to this element
    * that match the passed namespace URI.
    * 
    * @param nsURI the namespace uri for the elements.
    * @return a vector of elements.
    */
   public Vector getElementsNS(String nsURI)
   {
      Vector elements = new Vector();
      
      String prefix = (String)mNamespaceToPrefixMap.get(nsURI);
      if(prefix != null)
      {
         Iterator i = getElements().iterator();
         while(i.hasNext())
         {
            Element e = (Element)i.next();
            ElementReader er = new ElementReader(e);
            if(er.getTagName().startsWith(prefix + ":"))
            {
               elements.add(e);
            }
         }
      }
      
      return elements;
   }   
   
   /**
    * Gets a vector of elements that are children and
    * wrapped by element readers that match the passed tag name.
    * 
    * @param tagName the tag name of the elements.
    * @return a vector of element readers.
    */
   public Vector getElementReaders(String tagName)
   {
      Vector ereaders = new Vector();
      
      NodeList list = mElement.getChildNodes();
      for(int i = 0; i < list.getLength(); i++)
      {
         Node node = list.item(i);
         
         if(node.getNodeName().equals(tagName))
         {
            ereaders.add(new ElementReader((Element)list.item(i)));
         }
      }

      return ereaders;
   }
   
   /**
    * Gets a vector of elements that are children and
    * wrapped by element readers that match the passed namespace URI.
    * 
    * @param nsURI the name space URI of the elements.
    * @return a vector of element readers.
    */
   public Vector getElementReadersNS(String nsURI)
   {
      Vector ereaders = new Vector();
      
      String prefix = (String)mNamespaceToPrefixMap.get(nsURI);
      if(prefix != null)
      {
         Iterator i = getElementReaders().iterator();
         while(i.hasNext())
         {
            ElementReader er = (ElementReader)i.next();
            if(er.getTagName().startsWith(prefix + ":"))
            {
               ereaders.add(er);
            }
         }
      }
      
      return ereaders;
   }
   
   /**
    * Gets the first element encountered with the passed
    * tag name.
    * 
    * @param tagName the tag name of the element to look for.
    * @return the first element with the passed tag name or
    *         null if no element with the tag name could be found.
    */
   public Element getFirstElement(String tagName)
   {
      Element e = null;
      
      Vector elements = getElements(tagName);
      if(elements.size() > 0)
      {
         e = (Element)elements.get(0);
      }
      
      return e;
   }
   
   /**
    * Gets the first element wrapped as an element reader
    * encountered with the passed tag name.
    * 
    * @param tagName the tag name of the element to look for.
    * @return the first element with the passed tag name and
    *         wrapped by an element reader or null if no element
    *         with the tag name could be found.
    */
   public ElementReader getFirstElementReader(String tagName)
   {
      ElementReader ereader = null;
      
      Vector elements = getElements(tagName);
      if(elements.size() > 0)
      {
         ereader = new ElementReader((Element)elements.get(0));
      }
      
      return ereader;
   }   
   
   /**
    * Gets an attribute value as a string according to its name.
    * 
    * @param name the name of the attribute.
    * @return the value of the attribute as a string or the empty
    *         string if the attribute does not exist.
    */
   public String getStringAttribute(String name)
   {
      String rval = "";
      
      if(mElement != null)
      {
         rval = mElement.getAttribute(name);
      }
      else
      {
         LoggerManager.warning("dbcommon", "element doesn't exist!");
      }
      
      return rval;
   }
   
   /**
    * Gets an attribute value as a long according to its name.
    * 
    * @param name the name of the attribute.
    * @return the value of the attribute as a long or -1
    *         if the value was invalid or the attribute
    *         does not exist.
    */
   public long getLongAttribute(String name)
   {
      long l = -1;
      
      try
      {
         l = Long.parseLong(getStringAttribute(name));
      }
      catch(NumberFormatException nfe)
      {
      }
      
      return l;
   }
   
   /**
    * Gets an attribute value as an int according to its name.
    * 
    * @param name the name of the attribute.
    * @return the value of the attribute as an int or -1
    *         if the value was invalid or the attribute
    *         does not exist.
    */
   public int getIntAttribute(String name)
   {
      int i = -1;
      
      try
      {
         i = Integer.parseInt(getStringAttribute(name));
      }
      catch(NumberFormatException nfe)
      {
      }
      
      return i;
   }
   
   /**
    * Gets an attribute value as a float according to its name.
    * 
    * @param name the name of the attribute.
    * @return the value of the attribute as a float or -1
    *         if the value was invalid or the attribute
    *         does not exist.
    */
   public float getFloatAttribute(String name)
   {
      float f = -1;
      
      try
      {
         f = Float.parseFloat(getStringAttribute(name));
      }
      catch(NumberFormatException nfe)
      {
      }
      
      return f;
   }
   
   /**
    * Gets an attribute value as a boolean according to its name.
    * 
    * @param name the name of the attribute.
    * @return the value of the attribute as a boolean. False is returned
    *         by default.
    */
   public boolean getBooleanAttribute(String name)
   {
      boolean b = false;
      
      String s = getStringAttribute(name);
      int i = -1;
      try
      {
         i = Integer.parseInt(s);
         if(i != 0)
         {
            b = true;
         }
      }
      catch(NumberFormatException nfe)
      {
         if(s.compareToIgnoreCase("true") == 0)
         {
            b = true;
         }
      }
      
      return b;
   }
   
   /**
    * Gets the value of an element as a string.
    * 
    * @return the value of an element as a string.
    */
   public String getStringValue()
   {
      String s = "";
      
      Node node = mElement.getFirstChild();
      
      if(node != null)
      {
         s = node.getNodeValue();
      }
      
      return s;
   }
   
   /**
    * Gets the value of an element as a long.
    * 
    * @return the value of an element as a long or -1 if
    *         the value could not be parsed into a long.
    */
   public long getLongValue()
   {
      long l = -1;
      
      try
      {
         l = Long.parseLong(getStringValue());
      }
      catch(NumberFormatException nfe)
      {
      }
      
      return l;
   }
   
   /**
    * Gets the value of an element as an int.
    * 
    * @return the value of an element as an int or -1 if
    *         the value could not be parsed into an int.
    */
   public int getIntValue()
   {
      int i = -1;
      
      try
      {
         i = Integer.parseInt(getStringValue());
      }
      catch(NumberFormatException nfe)
      {
      }
      
      return i;
   }
   
   /**
    * Gets the value of an element as a float.
    * 
    * @return the value of an element as a float or -1 if
    *         the value could not be parsed into a long.
    */
   public float getFloatValue()
   {
      float f = -1;
      
      try
      {
         f = Float.parseFloat(getStringValue());
      }
      catch(NumberFormatException nfe)
      {
      }
      
      return f;
   }
   
   /**
    * Gets the string value of the first child element that matches
    * the passed tag name.
    * 
    * @param tagName the tag name of the element to look for.
    * @return the value of a child element as a string or an empty string
    *         if the child could not be found.
    */
   public String getFirstElementStringValue(String tagName)
   {
      String s = "";
      
      ElementReader ereader = getFirstElementReader(tagName);
      
      if(ereader != null)
      {
         s = ereader.getStringValue();
      }
      
      return s;
   }
   
   /**
    * Gets the boolean value of the first child element that matches
    * the passed tag name.
    * 
    * @param tagName the tag name of the element to look for.
    * @return the value of a child element as a boolean.
    */
   public boolean getFirstElementBooleanValue(String tagName)
   {
      boolean b = false;
      
      String s = getFirstElementStringValue(tagName);

      int i = -1;
      try
      {
         i = Integer.parseInt(s);
         if(i != 0)
         {
            b = true;
         }
      }
      catch(NumberFormatException nfe)
      {
         if(s.compareToIgnoreCase("true") == 0)
         {
            b = true;
         }
      }
      
      return b;   
   }
   
   /**
    * Gets the long value of the first child element that matches
    * the passed tag name.
    * 
    * @param tagName the tag name of the element to look for.
    * @return the value of a child element as a long or -1
    *         if the child could not be found.
    */
   public long getFirstElementLongValue(String tagName)
   {
      long l = -1;
      
      ElementReader ereader = getFirstElementReader(tagName);
      
      if(ereader != null)
      {
         l = ereader.getLongValue();
      }
      
      return l;
   }
   
   /**
    * Gets the int value of the first child element that matches
    * the passed tag name.
    * 
    * @param tagName the tag name of the element to look for.
    * @return the value of a child element as an int or -1
    *         if the child could not be found.
    */
   public int getFirstElementIntValue(String tagName)
   {
      int i = -1;
      
      ElementReader ereader = getFirstElementReader(tagName);
      
      if(ereader != null)
      {
         i = ereader.getIntValue();
      }
      
      return i;
   }
   
   /**
    * Gets the float value of the first child element that matches
    * the passed tag name.
    * 
    * @param tagName the tag name of the element to look for.
    * @return the value of a child element as a float or -1
    *         if the child could not be found.
    */
   public float getFirstElementFloatValue(String tagName)
   {
      float rval = -1;
      
      ElementReader ereader = getFirstElementReader(tagName);
      
      if(ereader != null)
      {
         rval = ereader.getFloatValue();
      }
      
      return rval;
   }   
   
   /**
    * Gets the tag name for the element wrapped by this reader.
    * 
    * @return the tag name of the element wrapped by this reader.
    */
   public String getTagName()
   {
      return mElement.getNodeName();
   }
   
   /**
    * Gets the element that this reader wraps.
    * 
    * @return the element that this reader wraps.
    */
   public Element getElement()
   {
      return mElement;
   }
}
