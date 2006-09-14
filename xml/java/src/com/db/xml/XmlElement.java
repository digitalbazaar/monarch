/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

import java.util.Collection;
import java.util.Iterator;
import java.util.Vector;

import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * An XmlElement stores information about an XML element. That includes
 * XML attributes and XML data and/or other XML elements as children. This
 * object is used for simplified, straight-forward, XML handling for classes
 * that are only meant to serialize their data to generic attributes and
 * and elements with ease.
 * 
 * It abstracts away specific XML details that objects that wish to serialize
 * to XML (and deserialize from) don't care about or want to deal with.
 * 
 * An XmlElement be parsed from an XML string or written out to an XML string.
 * 
 * FUTURE CODE: we need to better namespace support -- the way we check for
 * element names and attributes sometimes doesn't qualify them, which defeats
 * the purpose of namespaces.
 * 
 * @author Dave Longley
 */
public class XmlElement extends AbstractXmlSerializer
{
   /**
    * The name of this element.
    */
   protected String mName;
   
   /**
    * The namespace for this element.
    */
   protected String mNamespace;
   
   /**
    * The namespace URI for this element. This is the URI that this element's
    * namespace prefix maps to.
    */
   protected String mNamespaceUri;
   
   /**
    * The attributes for this element.
    */
   protected XmlAttributeMap mAttributes;
   
   /**
    * The parent of this element (can be null).
    */
   protected XmlElement mParent;
   
   /**
    * The children for this element.
    */
   protected Vector mChildren;
   
   /**
    * The data/value for this element.
    */
   protected String mData;
   
   /**
    * Creates a new blank XmlElement.
    */
   public XmlElement()
   {
      this("");
   }
   
   /**
    * Creates a new XmlElement with the specified name and default namespace.
    * 
    * @param name the name of this XmlElement -- this is the name that will
    *             be displayed inside of its root tag.
    */
   public XmlElement(String name)
   {
      this(name, null, null);
   }
   
   /**
    * Creates a new XmlElement with the specified name and namespace (the
    * prefix for the name).
    * 
    * @param name the name of this XmlElement -- this is the name that will
    *             be displayed inside of its root tag.
    * @param namespace the namespace of this XmlElement -- this is the prefix
    *                  that is displayed before the name of the element (null
    *                  indicates the default namespace where no prefix is
    *                  displayed before the name).
    * @param namespaceUri the URI (Universal Resource Indicator) that points
    *                     to the definition of the namespace for this element.
    */
   public XmlElement(String name, String namespace, String namespaceUri)
   {
      // create the vector for this element's children
      mChildren = new Vector();
      
      // create the attribute map for this element
      mAttributes = new XmlAttributeMap();
      
      // set the name of this element
      setName(name);
      
      // set the namespace of this element
      setNamespace(namespace, namespaceUri);
      
      // default this element's parent to null
      setParent(null);
      
      // default to no data
      setData(null);
   }
   
   /**
    * Resets this XmlElement's information.
    */
   protected void reset()
   {
      // reset the name of this element
      setName("");
      
      // reset the namespace of this element
      setNamespace(null, null);
      
      // clear the attributes of this element
      getAttributeMap().clear();
      
      // clear the children of this element
      clearChildren();
      
      // clear the data in this element
      setData(null);      
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return getName();
   }
   
   /**
    * Converts this object to XML.
    * 
    * @param header true to include an XML header, false not to.
    * @param indentSize the number of spaces to indent this element.
    * @param childIndentSize the number of additional spaces to indent
    *                        each child.
    * 
    * @return the XML for this object.
    */
   public String convertToXml(
      boolean header, int indentSize, int childIndentSize)
   {
      StringBuffer xml = new StringBuffer();

      // include header if appropriate
      if(header)
      {
         xml.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
      }
      
      // create indentation string
      StringBuffer indent = new StringBuffer();
      if(indentSize > 0)
      {
         indent.append("\n");
         for(int i = 0; i < indentSize; i++)
         {
            indent.append(' ');
         }
      }
      
      // start root tag
      xml.append(indent);
      xml.append("<" + getFullName());
      
      // see if this element has attributes
      XmlAttributeMap attributes = getAttributeMap();
      if(attributes.getAttributeCount() > 0)
      {
         for(Iterator i = attributes.getAttributeNames().iterator();
             i.hasNext();)
         {
            String name = (String)i.next();
            
            // encode and convert attribute
            xml.append(" " + name + "=\"");
            xml.append(XmlCoder.encode(attributes.getAttributeValue(name)));
            xml.append("\"");
         }
      }
      
      // see if this element has no children or data
      if(!hasChildren() && (getData() == null || getData().equals("")))
      {
         // end the start tag with an end tag
         xml.append("/>");
      }
      else
      {
         // end the start tag
         xml.append('>');
         
         // convert this element's children if it has any
         if(hasChildren())
         {
            // convert each child
            for(Iterator i = getChildren().iterator(); i.hasNext();)
            {
               XmlElement child = (XmlElement)i.next();
               xml.append(child.convertToXml(
                  false, indentSize + childIndentSize, childIndentSize));
            }
            
            if(indentSize > 0)
            {
               // add indentation
               xml.append(indent);
            }
            else if(childIndentSize > 0)
            {
               xml.append("\n");
            }
         }
         else
         {
            // encode and convert this element's data
            xml.append(XmlCoder.encode(getData()));
         }
         
         // add the end tag
         xml.append("</" + getFullName() + ">");
      }
      
      return xml.toString();
   }
   
   /**
    * This method takes a parsed DOM XML element and converts it
    * back into this object's representation.
    *
    * @param element the parsed element that contains this objects information.
    * 
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXml(Element element)
   {
      boolean rval = true;

      // reset this element's information
      reset();
      
      // get the tag name
      String name = element.getTagName();
      String namespace = null;
      
      // see if the name has a prefix
      int index = name.indexOf(":");
      if(index != -1)
      {
         // parse out the namespace and name
         namespace = name.substring(0, index);
         name = name.substring(index + 1);
      }
      
      // set name and namespace
      setName(name);
      
      // convert the attributes for this element
      XmlAttributeMap attributes = getAttributeMap();
      NamedNodeMap map = element.getAttributes();
      for(int i = 0; i < map.getLength(); i++)
      {
         Node attributeNode = map.item(i);
         
         // get the attribute name and namespace
         String attributeName = attributeNode.getNodeName();
         String attributeNamespace =
            XmlElement.getNamespacePrefix(attributeName);
         attributeName = XmlElement.getBasicName(attributeName);
         
         // add a namespace mapping if appropriate
         if(attributeNamespace != null && attributeNamespace.equals("xmlns"))
         {
            // add a namespace definition
            attributes.addNamespaceMapping(
               attributeName, attributeNode.getNodeValue());
         }
         
         // add attribute (XML decoding is handled automatically)
         attributes.addAttribute(
            attributeName, attributeNode.getNodeValue(),
            attributeNamespace, findNamespaceUri(attributeNamespace));
      }
      
      // set the namespace for this element now that the attributes have
      // been parsed
      setNamespace(namespace, findNamespaceUri(namespace));
      
      // convert the children for this element
      NodeList list = element.getChildNodes();
      for(int i = 0; i < list.getLength(); i++)
      {
         // children must be elements
         if(list.item(i) instanceof Element)
         {
            // create and convert child
            XmlElement child = new XmlElement();
            child.setParent(this);
            if(child.convertFromXml((Element)list.item(i)))
            {
               // add child
               addChild(child);
            }
         }
      }
      
      if(!hasChildren())
      {
         // the first child of the DOM element holds the element's data 
         Node dataNode = element.getFirstChild();
         if(dataNode != null)
         {
            // set data (XML decoding is handled automatically)
            setData(dataNode.getNodeValue());
         }
      }
      
      return rval;
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
      // set parent and return this element
      setParent(parent);
      return this;
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
      // reset this element's information
      reset();
      
      // copy the passed element's information
      setName(element.getName());
      setParent(element.getParent());
      setNamespace(element.getNamespace(), element.getNamespaceUri());
      
      // copy attributes
      for(Iterator i = element.getAttributeMap().getAttributeNames().iterator();
          i.hasNext();)
      {
         String name = (String)i.next();
         String value = element.getAttributeMap().getAttributeValue(name);
         getAttributeMap().addAttribute(name, value);
      }
      
      // copy children
      for(Iterator i = element.getChildren().iterator(); i.hasNext();)
      {
         XmlElement child = (XmlElement)i.next();
         
         // create a new child and convert it
         XmlElement newChild = new XmlElement();
         newChild.convertFromXmlElement(child);
         
         // add the new child
         addChild(newChild);
      }
      
      // copy data
      setData(element.getData());
      
      // always true
      return true;
   }
   
   /**
    * Sets the name of this XmlElement. This will set the base name
    * for the XmlElement -- it will not include the namespace of this
    * element.
    * 
    * @param name the name of this XmlElement.
    */
   public void setName(String name)
   {
      if(name == null)
      {
         name = "";
      }
      
      mName = name;
   }
   
   /**
    * Gets the name of this XmlElement. This will return the base name
    * for the XmlElement -- it will not include the namespace of this
    * element.
    * 
    * @return the name of this XmlElement.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Gets the full name of this XmlElement including its namespace. This
    * will return the namespace of this element, followed by a colon,
    * followed by the local name for the XmlElement.
    * 
    * @return the name of this XmlElement.
    */
   public String getFullName()
   {
      String rval = getName();
      
      if(getNamespace() != null && !getNamespace().equals(""))
      {
         rval = getNamespace() + ":" + getName();
      }
      
      return rval;
   }   
   
   /**
    * Sets the namespace of this XmlElement.
    * 
    * @param namespace the namespace of this XmlElement.
    * @param namespaceUri the URI that points to the definition for the
    *                     namespace.
    */
   public void setNamespace(String namespace, String namespaceUri)
   {
      mNamespace = namespace;
      mNamespaceUri = namespaceUri;
      
      // set the namespace for all children, if not already set
      for(Iterator i = getChildren().iterator(); i.hasNext();)
      {
         XmlElement child = (XmlElement)i.next();
         if(child.getNamespace() == null)
         {
            child.setNamespace(namespace, namespaceUri);
         }
      }
   }
   
   /**
    * Gets the namespace of this XmlElement.
    * 
    * @return the namespace of this XmlElement.
    */
   public String getNamespace()
   {
      return mNamespace;
   }
   
   /**
    * Gets the namespace URI of this XmlElement.
    * 
    * @return the namespace URI of this XmlElement.
    */
   public String getNamespaceUri()
   {
      return mNamespaceUri;
   }
   
   /**
    * Finds the namespace prefix for the given namespace URI by looking
    * first through this element's attributes for a mapping, and then looking
    * up its parent tree for a mapping.
    * 
    * @param namespaceUri the namespace URI to find the namespace prefix for.
    * 
    * @return the namespace prefix for the given namespace URI or null if the
    *         prefix could not be found.
    */
   public String findNamespace(String namespaceUri)
   {
      String rval = null;
      
      if(namespaceUri != null)
      {
         // go through this element's attributes to find the namespace URI
         boolean found = false;
         for(Iterator i = getAttributeMap().getAttributeNames().iterator();
             i.hasNext() && !found;)
         {
            String name = (String)i.next();
            
            String value = getAttributeValue(name);
            if(value.equals(namespaceUri))
            {
               rval = getAttributeMap().getNamespace(namespaceUri);
               found = true;
            }
         }
         
         // if the mapping was not found, check this element's namespace URI
         // to see if it matches the passed URI
         if(!found && getNamespaceUri() != null &&
            getNamespaceUri().equals(namespaceUri)) 
         {
            rval = getNamespace();
            found = true;
         }
         
         // if the mapping was not found at this level, look up the parent tree
         if(!found && getParent() != null) 
         {
            rval = getParent().findNamespace(namespaceUri);
         }
      }
      
      return rval;
   }
   
   /**
    * Finds the namespace URI for the given namespace prefix by looking
    * first through this element's attributes for a mapping, and then looking
    * up its parent tree for a mapping.
    * 
    * @param namespace the namespace prefix to find the namespace URI for.
    * 
    * @return the namespace URI for the given namespace prefix or null if the
    *         URI could not be found.
    */
   public String findNamespaceUri(String namespace)
   {
      String rval = null;
      
      if(namespace != null)
      {
         // go through this element's attributes to find the namespace
         boolean found = false;
         for(Iterator i = getAttributeMap().getAttributeNames().iterator();
             i.hasNext() && !found;)
         {
            String name = (String)i.next();
            
            if(name.equals(namespace) || name.equals("xmlns:" + namespace))
            {
               rval = getAttributeValue(name);
               found = true;
            }
         }
         
         // if the mapping was not found, check this element's namespace
         // to see if it matches the passed one
         if(!found && getNamespace() != null &&
            getNamespace().equals(namespace)) 
         {
            rval = getNamespaceUri();
            found = true;
         }
         
         // if the mapping was not found at this level, look up the parent tree
         if(!found && getParent() != null) 
         {
            rval = getParent().findNamespaceUri(namespace);
         }
      }
      
      return rval;
   }   
   
   /**
    * Gets the attribute map for this XmlElement. This map can be used to
    * add or remove attributes from this XmlElement.
    *
    * @return the attribute map for this XmlElement.
    */
   public XmlAttributeMap getAttributeMap()
   {
      return mAttributes;
   }
   
   /**
    * A convenience method for adding an attribute to this XmlElement's
    * attribute map.
    * 
    * @param name the name of the attribute to add.
    * @param value the value of the attribute to add.
    */
   public void addAttribute(String name, String value)
   {
      getAttributeMap().addAttribute(name, value);
   }
   
   /**
    * A convenience method for adding an attribute to this XmlElement's
    * attribute map.
    * 
    * @param name the name of the attribute to add.
    * @param value the value of the attribute to add.
    */
   public void addAttribute(String name, int value)
   {
      getAttributeMap().addAttribute(name, value);
   }
   
   /**
    * A convenience method for adding an attribute to this XmlElement's
    * attribute map.
    * 
    * @param name the name of the attribute to add.
    * @param value the value of the attribute to add.
    */
   public void addAttribute(String name, long value)
   {
      getAttributeMap().addAttribute(name, value);
   }
   
   /**
    * A convenience method for adding an attribute to this XmlElement's
    * attribute map.
    * 
    * @param name the name of the attribute to add.
    * @param value the value of the attribute to add.
    */
   public void addAttribute(String name, float value)
   {
      getAttributeMap().addAttribute(name, value);
   }
   
   /**
    * A convenience method for adding an attribute to this XmlElement's
    * attribute map.
    * 
    * @param name the name of the attribute to add.
    * @param value the value of the attribute to add.
    */
   public void addAttribute(String name, double value)
   {
      getAttributeMap().addAttribute(name, value);
   }
   
   /**
    * A convenience method for adding an attribute to this XmlElement's
    * attribute map.
    * 
    * @param name the name of the attribute to add.
    * @param value the value of the attribute to add.
    */
   public void addAttribute(String name, boolean value)
   {
      getAttributeMap().addAttribute(name, value);
   }
   
   /**
    * A convenience method for adding an attribute to this XmlElement's
    * attribute map that has a special namespace.
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
      getAttributeMap().addAttribute(name, value, namespace, namespaceUri);
   }
   
   /**
    * A convenience method for getting an attribute value from this
    * XmlElement's attribute map.
    * 
    * @param name the name of the attribute to get the value of.
    * 
    * @return the value of the attribute as a string, or a blank string if the
    *         attribute does not exist.
    */
   public String getAttributeValue(String name)
   {
      return getAttributeValue(name, null);
   }
   
   /**
    * A convenience method for getting an attribute value from this
    * XmlElement's attribute map.
    * 
    * @param name the name of the attribute to get the value of.
    * @param namespaceUri the namespace uri for the name.
    * 
    * @return the value of the attribute as a string, or a blank string if the
    *         attribute does not exist.
    */
   public String getAttributeValue(String name, String namespaceUri)
   {
      String prefix = findNamespace(namespaceUri);
      if(prefix != null)
      {
         // get full name
         name = prefix + ":" + name;
      }
      
      return getAttributeMap().getAttributeValue(name);
   }
   
   /**
    * A convenience method for getting an attribute value from this
    * XmlElement's attribute map.
    * 
    * @param name the name of the attribute to get the value of.
    * 
    * @return the value of the attribute as an int, or 0 if the attribute
    *         could not be parsed as such or it didn't exist.
    */
   public int getAttributeIntValue(String name)
   {
      return getAttributeMap().getAttributeIntValue(name);
   }
   
   /**
    * A convenience method for getting an attribute value from this
    * XmlElement's attribute map.
    * 
    * @param name the name of the attribute to get the value of.
    * 
    * @return the value of the attribute as a long, or 0 if the attribute
    *         could not be parsed as such or it didn't exist.
    */
   public long getAttributeLongValue(String name)
   {
      return getAttributeMap().getAttributeLongValue(name);
   }
   
   /**
    * A convenience method for getting an attribute value from this
    * XmlElement's attribute map.
    * 
    * @param name the name of the attribute to get the value of.
    * 
    * @return the value of the attribute as a float, or 0 if the attribute
    *         could not be parsed as such or it didn't exist.
    */
   public float getAttributeFloatValue(String name)
   {
      return getAttributeMap().getAttributeFloatValue(name);
   }
   
   /**
    * A convenience method for getting an attribute value from this
    * XmlElement's attribute map.
    * 
    * @param name the name of the attribute to get the value of.
    * 
    * @return the value of the attribute as a double, or 0 if the attribute
    *         could not be parsed as such or it didn't exist.
    */
   public double getAttributeDoubleValue(String name)
   {
      return getAttributeMap().getAttributeDoubleValue(name);
   }
   
   /**
    * A convenience method for getting an attribute value from this
    * XmlElement's attribute map.
    * 
    * @param name the name of the attribute to get the value of.
    * 
    * @return the value of the attribute as a boolean, or false if the
    *         attribute could not be parsed as such or it didn't exist.
    */
   public boolean getAttributeBooleanValue(String name)
   {
      return getAttributeMap().getAttributeBooleanValue(name);
   }   
   
   /**
    * Sets the data for this XmlElement.
    * 
    * @param data the data for this XmlElement.
    */
   public void setData(String data)
   {
      mData = data;
   }
   
   /**
    * Gets the data for this XmlElement.
    * 
    * @return the data for this XmlElement.
    */
   public String getData()
   {
      String rval = "";
      
      if(mData != null)
      {
         rval = mData;
      }
      
      return rval;
   }
   
   /**
    * Sets the value for this XmlElement. This method is an alias for setData().
    * 
    * @param value the value for this XmlElement.
    */
   public void setValue(String value)
   {
      setData(value);
   }
   
   /**
    * Sets the value for this XmlElement. This method is an alias for setData().
    * 
    * @param value the value for this XmlElement.
    */
   public void setValue(int value)
   {
      setValue("" + value);
   }
   
   /**
    * Sets the value for this XmlElement. This method is an alias for setData().
    * 
    * @param value the value for this XmlElement.
    */
   public void setValue(long value)
   {
      setValue("" + value);
   }
   
   /**
    * Sets the value for this XmlElement. This method is an alias for setData().
    * 
    * @param value the value for this XmlElement.
    */
   public void setValue(float value)
   {
      setValue("" + value);
   }
   
   /**
    * Sets the value for this XmlElement. This method is an alias for setData().
    * 
    * @param value the value for this XmlElement.
    */
   public void setValue(double value)
   {
      setValue("" + value);
   }
   
   /**
    * Sets the value for this XmlElement. This method is an alias for setData().
    * 
    * @param value the value for this XmlElement.
    */
   public void setValue(boolean value)
   {
      setValue("" + value);
   }
   
   /**
    * Gets the value for this XmlElement. This method is an alias for getData().
    * 
    * @return the value for this XmlElement.
    */
   public String getValue()
   {
      return getData();
   }
   
   /**
    * Gets the value for this XmlElement as an int.
    * 
    * @return the value for this XmlElement or 0 if the value cannot be parsed.
    */
   public int getIntValue()
   {
      int rval = 0;
      
      try
      {
         rval = Integer.parseInt(getValue());
      }
      catch(Throwable ignore)
      {
      }
      
      return rval;
   }
   
   /**
    * Gets the value for this XmlElement as a long.
    * 
    * @return the value for this XmlElement or 0 if the value cannot be parsed.
    */
   public long getLongValue()
   {
      long rval = 0;
      
      try
      {
         rval = Long.parseLong(getValue());
      }
      catch(Throwable ignore)
      {
      }
      
      return rval;
   }
   
   /**
    * Gets the value for this XmlElement as a float.
    * 
    * @return the value for this XmlElement or 0 if the value cannot be parsed.
    */
   public float getFloatValue()
   {
      float rval = 0;
      
      try
      {
         rval = Float.parseFloat(getValue());
      }
      catch(Throwable ignore)
      {
      }
      
      return rval;
   }
   
   /**
    * Gets the value for this XmlElement as a double.
    * 
    * @return the value for this XmlElement or 0 if the value cannot be parsed.
    */
   public double getDoubleValue()
   {
      double rval = 0;
      
      try
      {
         rval = Double.parseDouble(getValue());
      }
      catch(Throwable ignore)
      {
      }
      
      return rval;
   }
   
   /**
    * Gets the value for this XmlElement as a boolean.
    * 
    * @return the value for this XmlElement or false if the value cannot be
    *         parsed.
    */
   public boolean getBooleanValue()
   {
      boolean rval = false;
      
      try
      {
         rval = Boolean.parseBoolean(getValue());
      }
      catch(Throwable ignore)
      {
      }
      
      return rval;
   }   
   
   /**
    * Adds a child to this XmlElement.
    * 
    * @param child the XmlElement to add to this XmlElement's list of children.
    */
   public void addChild(XmlElement child)
   {
      // set child parent to this element
      child.setParent(this);
      
      // add the child to the list of children
      if(!mChildren.contains(child))
      {
         mChildren.add(child);
      }
   }
   
   /**
    * Removes a child from this XmlElement.
    * 
    * @param child the XmlElement to remove from this XmlElement's list
    *        of children.
    */
   public void removeChild(XmlElement child)
   {
      // remove thee child from the list of children
      if(mChildren.remove(child))
      {
         // set child parent to null
         child.setParent(null);
      }
   }
   
   /**
    * Clears the children from this XmlElement.
    */
   public void clearChildren()
   {
      mChildren.clear();
   }
   
   /**
    * Gets the child at the given index from this XmlElement.
    * 
    * @param index the index of the child to retrieve or null if the index
    *              is out of range.
    *              
    * @return the child at the given index, or null if the index is out of
    *         range.
    */
   public XmlElement getChildAt(int index)
   {
      XmlElement rval = null;
      
      if(index >= 0 && index < getChildCount())
      {
         rval = (XmlElement)mChildren.get(index);
      }
      
      return rval;
   }

   /**
    * Gets the first child of this XmlElement. 
    * 
    * @return the first child of this XmlElement or null if this XmlElement
    *         does not have a first child.
    */
   public XmlElement getFirstChild()
   {
      return getFirstChild(null);
   }
   
   /**
    * Gets the first child of this XmlElement with the specified name.
    * 
    * @param name the name of the child to retrieve.
    * 
    * @return the first child of this XmlElement with the specified name or
    *         null if this XmlElement doesn't have a first child with the
    *         passed name.
    */
   public XmlElement getFirstChild(String name)
   {
      XmlElement rval = null;
      
      if(name == null)
      {
         rval = getChildAt(0);
      }
      else
      {
         for(Iterator i = mChildren.iterator(); i.hasNext() && rval == null;)
         {
            XmlElement child = (XmlElement)i.next();
            if(child.getName().equals(name))
            {
               rval = child;
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the first child of this XmlElement with the specified name and
    * namespace.
    * 
    * @param name the name of the child to retrieve.
    * @param namespace the namespace of the child to retrieve.
    * 
    * @return the first child of this XmlElement with the specified name or
    *         null if this XmlElement doesn't have a first child with the
    *         passed name.
    */
   public XmlElement getFirstChild(String name, String namespace)
   {
      XmlElement rval = null;
      
      if(name == null && namespace == null)
      {
         rval = getChildAt(0);
      }
      else
      {
         for(Iterator i = mChildren.iterator(); i.hasNext() && rval == null;)
         {
            XmlElement child = (XmlElement)i.next();
            if(name == null)
            {
               if(child.getNamespace().equals(namespace))
               {
                  rval = child;
               }
            }
            else if(child.getName().equals(name))
            {
               if(namespace == null)
               {
                  rval = child;
               }
               else if(child.getNamespace().equals(namespace))
               {
                  rval = child;
               }
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the value of the first child of this XmlElement with the specified
    * name.
    * 
    * @param name the name of the child with the value to retrieve.
    * 
    * @return the value of the first child of this XmlElement with the
    *         specified name or a blank string if this XmlElement doesn't
    *         have a first child with the passed name or the child has no data.
    */
   public String getFirstChildValue(String name)
   {
      String rval = "";
      
      // get the first child
      XmlElement child = getFirstChild(name);
      if(child != null)
      {
         rval = child.getValue();
      }
      
      return rval;
   }
   
   /**
    * Gets the value of the first child of this XmlElement with the specified
    * name and namespace.
    * 
    * @param name the name of the child with the value to retrieve.
    * @param namespace the namespace of the child with the value to retrieve.
    * 
    * @return the value of the first child of this XmlElement with the
    *         specified name or a blank string if this XmlElement doesn't
    *         have a first child with the passed name or the child has no data.
    */
   public String getFirstChildValue(String name, String namespace)
   {
      String rval = "";
      
      // get the first child
      XmlElement child = getFirstChild(name, namespace);
      if(child != null)
      {
         rval = child.getValue();
      }
      
      return rval;
   }   
   
   /**
    * Gets the children for this XmlElement. Any changes to the returned
    * collection will be reflected in this XmlElement.
    * 
    * @return the children for this XmlElement in a collection of XmlElements.
    */
   public Collection getChildren()
   {
      return mChildren;
   }
   
   /**
    * Gets the children for this XmlElement with the specified name. Any
    * changes to the children in the returned collection will be reflected
    * in this XmlElement.
    * 
    * @param name the name of the children to return.
    * 
    * @return the children with the specified name for this XmlElement in
    *         a collection of XmlElements.
    */
   public Collection getChildren(String name)
   {
      Vector rval = new Vector();
      
      for(Iterator i = mChildren.iterator(); i.hasNext();)
      {
         XmlElement child = (XmlElement)i.next();
         if(child.getName().equals(name))
         {
            rval.add(child);
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the children for this XmlElement with the specified namespace. Any
    * changes to the children in the returned collection will be reflected in
    * this XmlElement.
    * 
    * @param namespace the namespace of the children to return.
    * 
    * @return the children with the specified namespace for this XmlElement in
    *         a collection of XmlElements.
    */
   public Collection getChildrenInNamespace(String namespace)
   {
      Vector rval = new Vector();
      
      for(Iterator i = mChildren.iterator(); i.hasNext();)
      {
         XmlElement child = (XmlElement)i.next();
         if(child.getNamespace().equals(namespace))
         {
            rval.add(child);
         }
      }
      
      return rval;
   }
      
   /**
    * Gets the children for this XmlElement with the specified name and
    * namespace. Any changes to the children in the returned collection will
    * be reflected in this XmlElement.
    * 
    * @param name the name of the children to return.
    * @param namespace the namespace of the children to return.
    * 
    * @return the children with the specified name and namespace for this
    *         XmlElement in a collection of XmlElements.
    */
   public Collection getChildren(String name, String namespace)
   {
      Vector rval = new Vector();
      
      for(Iterator i = mChildren.iterator(); i.hasNext();)
      {
         XmlElement child = (XmlElement)i.next();
         if(child.getName().equals(name) &&
            child.getNamespace().equals(namespace))
         {
            rval.add(child);
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the number of children this XmlElement has.
    * 
    * @return the number of children this XmlElement has.
    */
   public int getChildCount()
   {
      return mChildren.size();
   }
   
   /**
    * Returns true if this XmlElement has children, false if not.
    * 
    * @return true if this XmlElement has children, false if not.
    */
   public boolean hasChildren()
   {
      return getChildCount() > 0;
   }   
   
   /**
    * Sets the parent for this XmlElement. The parent is an XmlElement.
    * 
    * @param parent the parent XmlElement for this element (can be null). 
    */
   public void setParent(XmlElement parent)
   {
      mParent = parent;
   }

   /**
    * Gets the parent for this XmlElement. The parent is an XmlElement.
    * 
    * @return the parent XmlElement for this XmlElement. 
    */
   public XmlElement getParent()
   {
      return mParent;
   }
   
   /**
    * Gets the logger for this xml serializer.
    * 
    * @return the logger for this xml serializer.
    */
   public Logger getLogger()   
   {
      return LoggerManager.getLogger("dbxml");
   }
   
   /**
    * Gets the namespace prefix for the given name. 
    *
    * @param name the name to get the namespace prefix for.
    * 
    * @return the namespace for the name, or null if none exists.
    */
   public static String getNamespacePrefix(String name)
   {
      String namespace = null;
      
      // split the name to get the namespace, if any
      int index = name.indexOf(":");
      if(index != -1)
      {
         // get the namespace
         namespace = name.substring(0, index);
      }
      
      return namespace;
   }
   
   /**
    * Gets the basic name for the given fully qualified name (strips off
    * the namespace prefix, if any). 
    *
    * @param name the name to get the basic name for.
    * 
    * @return the basic name.
    */
   public static String getBasicName(String name)
   {
      String rval = name;
      
      // split the name to get the namespace, if any
      int index = name.indexOf(":");
      if(index != -1)
      {
         // get the basic name
         rval = name.substring(index + 1);
      }
      
      return rval;
   }
}
