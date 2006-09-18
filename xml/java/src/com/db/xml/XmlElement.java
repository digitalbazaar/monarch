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
 * This class is essentially a simpler/dumber implementation of a DOM element
 * that provides a less complex interface for manipulating XML. XmlElements are
 * constructed from DOM elements when converting from XML.
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
    * The namespace URI for this element. This is the URI that points to
    * the definition for this element.
    */
   protected String mNamespaceUri;
   
   /**
    * The list of XmlAttributes for this element.
    */
   protected XmlAttributeList mAttributes;
   
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
    * True to inherit this XmlElement's namespace URI from its parent
    * XmlElement if the namespace URI is set to null, false not to. 
    */
   protected boolean mInheritNamespaceUri;   
   
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
      this(name, null);
   }
   
   /**
    * Creates a new XmlElement with the specified name and namespace (the
    * prefix for the name).
    * 
    * @param name the name of this XmlElement -- this is the name that will
    *             be displayed inside of its root tag.
    * @param namespaceUri the URI (Universal Resource Indicator) that points
    *                     to the definition of the namespace for this element.
    */
   public XmlElement(String name, String namespaceUri)
   {
      this(name, namespaceUri, true);
   }
   
   /**
    * Creates a new XmlElement with the specified name and namespace (the
    * prefix for the name).
    * 
    * @param name the name of this XmlElement -- this is the name that will
    *             be displayed inside of its root tag.
    * @param namespaceUri the URI (Universal Resource Indicator) that points
    *                     to the definition of the namespace for this element.
    * @param inherit true to inherit this XmlElement's namespace URI from
    *                its parent XmlElement if the namespace URI is set to
    *                null, false not to.
    */
   public XmlElement(String name, String namespaceUri, boolean inherit)
   {
      // create the vector for this element's children
      mChildren = new Vector();
      
      // create the attribute list for this element
      mAttributes = new XmlAttributeList(this);
      
      // set the name of this element
      setName(name);
      
      // set the namespace URI of this element
      setNamespaceUri(namespaceUri);
      
      // set whether or not inherit namespace URI
      setInheritNamespaceUri(inherit);
      
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
      
      // reset the namespace URI of this element
      setNamespaceUri(null);
      
      // clear the attributes of this element
      getAttributeList().clear();
      
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
      xml.append("<" + getQualifiedName());
      
      // add this element's attributes
      for(Iterator i = getAttributeList().getAttributes().iterator();
          i.hasNext();)
      {
         XmlAttribute attribute = (XmlAttribute)i.next();
         
         // encode and convert attribute
         xml.append(" " + attribute.getQualifiedName() + "=\"");
         xml.append(XmlCoder.encode(attribute.getValue()));
         xml.append("\"");
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
         xml.append("</" + getQualifiedName() + ">");
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
      
      // get the namespace prefix and local name
      String namespacePrefix = parseNamespacePrefix(name);
      String localName = parseLocalName(name);
      
      // set name
      setName(localName);
      
      // do not inherit namespace URI
      setInheritNamespaceUri(false);
      
      // add the namespace mappings for this element's attributes first
      NamedNodeMap map = element.getAttributes();
      for(int i = 0; i < map.getLength(); i++)
      {
         Node attributeNode = map.item(i);
         
         // get the attribute name and namespace prefix
         String attributeName = attributeNode.getNodeName();
         
         if(attributeName.equals("xmlns") || attributeName.startsWith("xmlns:"))
         {
            // parse the attribute local name
            String attributeLocalName =
               XmlElement.parseLocalName(attributeName);
            if(attributeLocalName.equals("xmlns"))
            {
               // add null -> namespace URI mapping (a null namespace prefix
               // will point to the default namespace URI in this scope)
               getAttributeList().addNamespaceMapping(
                  null, attributeNode.getNodeValue());
            }
            else
            {
               // defining a new namespace, with a specified prefix as the
               // local name, so add namespace prefix -> namespace URI mapping
               getAttributeList().addNamespaceMapping(
                  attributeLocalName, attributeNode.getNodeValue());
            }
         }
      }
      
      // convert the attributes for this element
      for(int i = 0; i < map.getLength(); i++)
      {
         Node attributeNode = map.item(i);
         
         // get the attribute name and namespace prefix
         String attributeName = attributeNode.getNodeName();
         String attributeNamespaceUri = null;
         
         if(!attributeName.equals("xmlns") &&
            !attributeName.startsWith("xmlns:"))
         {
            // get the attribute namespace prefix
            String attributeNamespacePrefix =
               XmlElement.parseNamespacePrefix(attributeName);
            
            // find the namespace URI for this attribute
            attributeNamespaceUri = getAttributeList().findNamespaceUri(
               attributeNamespacePrefix);
            
            // use the local name as the attribute name
            attributeName = XmlElement.parseLocalName(attributeName);
         }
         
         // add attribute (XML decoding is handled automatically), do not
         // inherit namespace -- it should be set properly
         getAttributeList().addAttribute(
            attributeName, attributeNode.getNodeValue(),
            attributeNamespaceUri, false);
      }
      
      // set the namespace URI for this element now that the attributes have
      // been parsed
      setNamespaceUri(findNamespaceUri(namespacePrefix));
      
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
      setNamespaceUri(element.getNamespaceUri());
      setInheritNamespaceUri(element.getInheritNamespaceUri());
      
      // copy attributes
      for(Iterator i = element.getAttributeList().getAttributes().iterator();
          i.hasNext();)
      {
         XmlAttribute attribute = (XmlAttribute)i.next();
         
         // determine attribute namespace URI
         String attributeNamespaceUri = null;
         if(!attribute.getName().equals("xmlns") &&
            !attribute.getName().startsWith("xmlns:"))
         {
            attributeNamespaceUri = attribute.getNamespaceUri();
         }
         
         addAttribute(
            attribute.getName(), attribute.getValue(),
            attributeNamespaceUri, attribute.getInheritNamespaceUri());
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
    * Sets the name of this XmlElement. This will set the local name for
    * the XmlElement -- it will not include the namespace prefix of this
    * element.
    * 
    * @param name the local name of this XmlElement.
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
    * Gets the name of this XmlElement. This will return the local name
    * for the XmlElement -- it will not include the namespace prefix of this
    * element.
    * 
    * @return the local name of this XmlElement.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Gets the qualified name of this XmlElement. This includes its namespace
    * prefix, followed by a colon, followed by its local name.
    * 
    * @return the qualified name of this XmlElement.
    */
   public String getQualifiedName()
   {
      String rval = getName();
      
      String prefix = findNamespacePrefix(getNamespaceUri());
      
      if(prefix != null && !prefix.equals(""))
      {
         rval = prefix + ":" + getName();
      }
      
      return rval;
   }
   
   /**
    * Sets the namespace URI of this XmlElement.
    * 
    * @param namespaceUri the URI that points to the definition for the
    *                     namespace.
    */
   public void setNamespaceUri(String namespaceUri)
   {
      mNamespaceUri = namespaceUri;
   }
   
   /**
    * Gets the namespace URI of this XmlElement.
    * 
    * @return the namespace URI of this XmlElement.
    */
   public String getNamespaceUri()
   {
      String rval = mNamespaceUri;
      
      if(getInheritNamespaceUri() &&
         mNamespaceUri == null && getParent() != null)
      {
         // get parent's namespace URI
         rval = getParent().getNamespaceUri();
      }
      
      return rval;
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
   public String findNamespacePrefix(String namespaceUri)
   {
      String rval = null;
      
      if(namespaceUri != null)
      {
         // see if the namespace prefix is defined in this scope
         if(getAttributeList().isNamespacePrefixDefined(namespaceUri))
         {
            // find the namespace prefix
            rval = getAttributeList().findNamespacePrefix(namespaceUri);
         }
         else
         {
            // look up the parent tree for the prefix, if possible
            if(getParent() != null) 
            {
               rval = getParent().findNamespacePrefix(namespaceUri);
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Finds the namespace URI for the given namespace prefix by looking
    * first through this element's attributes for a mapping, and then looking
    * up its parent tree for a mapping.
    * 
    * @param namespacePrefix the namespace prefix to find the namespace URI for.
    * 
    * @return the namespace URI for the given namespace prefix or null if the
    *         URI could not be found.
    */
   public String findNamespaceUri(String namespacePrefix)
   {
      String rval = null;
      
      // see if the namespace URI is defined at this scope
      if(getAttributeList().isNamespaceUriDefined(namespacePrefix))
      {
         // find the namespace URI
         rval = getAttributeList().findNamespaceUri(namespacePrefix);
      }
      else
      {
         // look up the parent tree for the URI, if possible
         if(getParent() != null) 
         {
            rval = getParent().findNamespaceUri(namespacePrefix);
         }
      }
      
      return rval;
   }   
   
   /**
    * Gets the attribute list for this XmlElement. This list can be used to
    * add or remove attributes from this XmlElement.
    *
    * @return the attribute list for this XmlElement.
    */
   public XmlAttributeList getAttributeList()
   {
      return mAttributes;
   }
   
   /**
    * A convenience method for adding an attribute to this XmlElement's
    * attribute list.
    * 
    * @param name the name of the attribute to add.
    * @param value the value of the attribute to add.
    */
   public void addAttribute(String name, String value)
   {
      getAttributeList().addAttribute(name, value);
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
      getAttributeList().addAttribute(name, value);
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
      getAttributeList().addAttribute(name, value);
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
      getAttributeList().addAttribute(name, value);
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
      getAttributeList().addAttribute(name, value);
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
      getAttributeList().addAttribute(name, value);
   }
   
   /**
    * A convenience method for adding an attribute to this XmlElement's
    * attribute map that has a special namespace.
    * 
    * @param name the name of the attribute.
    * @param value the value of the attribute.
    * @param namespaceUri the URI that points to the definition of the
    *                     namespace for the attribute.
    */
   public void addAttribute(
      String name, String value, String namespaceUri)
   {
      getAttributeList().addAttribute(name, value, namespaceUri, true);
   }
   
   /**
    * A convenience method for adding an attribute to this XmlElement's
    * attribute map that has a special namespace.
    * 
    * @param name the name of the attribute.
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
      getAttributeList().addAttribute(name, value, namespaceUri, inherit);
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
      String rval = null;
      
      if(name.equals("xmlns") || name.startsWith("xmlns:"))
      {
         rval = getAttributeValue(name, null);
      }
      else
      {
         rval = getAttributeValue(name, getNamespaceUri());
      }
      
      return rval;
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
      return getAttributeList().getAttributeIntValue(name);
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
      return getAttributeList().getAttributeLongValue(name);
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
      return getAttributeList().getAttributeFloatValue(name);
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
      return getAttributeList().getAttributeDoubleValue(name);
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
      return getAttributeList().getAttributeBooleanValue(name);
   }
   
   /**
    * A convenience method for getting an attribute value from this
    * XmlElement's attribute map.
    * 
    * @param name the name of the attribute to get the value of.
    * @param namespaceUri the namespace uri for the attribute.
    * 
    * @return the value of the attribute as a string, or a blank string if the
    *         attribute does not exist.
    */
   public String getAttributeValue(String name, String namespaceUri)
   {
      return getAttributeList().getAttributeValue(name, namespaceUri);
   }
   
   /**
    * A convenience method for checking whether or not this XmlElement has
    * a particular attribute in its namespace.
    * 
    * @param name the local name for the attribute (unless the attribute is
    *        used to define a namespace with "xmlns" in which case the name
    *        should be the fully qualified name).
    * 
    * @return true if this XmlElement has an attribute with the given name,
    *         false if not.
    */
   public boolean hasAttribute(String name)
   {
      boolean rval = false;

      if(name.equals("xmlns") || name.startsWith("xmlns:"))
      {
         rval = getAttributeList().hasAttribute(name, null);
      }
      else
      {
         rval = getAttributeList().hasAttribute(name, getNamespaceUri());
      }
      
      return rval;
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
      XmlElement rval = null;
      
      if(hasChildren())
      {
         rval = getChildAt(0);
      }
      
      return rval;
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
      
      for(Iterator i = mChildren.iterator(); i.hasNext() && rval == null;)
      {
         XmlElement child = (XmlElement)i.next();
         if(child.getName().equals(name))
         {
            rval = child;
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the first child of this XmlElement with the specified name and
    * namespace URI.
    * 
    * @param name the name of the child to retrieve.
    * @param namespaceUri the namespace URI of the child to retrieve.
    * 
    * @return the first child of this XmlElement with the specified name or
    *         null if this XmlElement doesn't have a first child with the
    *         passed name.
    */
   public XmlElement getFirstChild(String name, String namespaceUri)
   {
      XmlElement rval = null;
      
      for(Iterator i = mChildren.iterator(); i.hasNext() && rval == null;)
      {
         XmlElement child = (XmlElement)i.next();
         String childNamespaceUri = child.getNamespaceUri();
         
         // check name
         if(child.getName().equals(name))
         {
            // check namespace URI
            if((childNamespaceUri == null && namespaceUri == null) ||
               (childNamespaceUri != null &&
                childNamespaceUri.equals(namespaceUri)))
            {
               rval = child;
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
    * name and namespace URI.
    * 
    * @param name the name of the child with the value to retrieve.
    * @param namespaceUri the namespace URI of the child with the value to
    *                     retrieve.
    * 
    * @return the value of the first child of this XmlElement with the
    *         specified name or a blank string if this XmlElement doesn't
    *         have a first child with the passed name or the child has no data.
    */
   public String getFirstChildValue(String name, String namespaceUri)
   {
      String rval = "";
      
      // get the first child
      XmlElement child = getFirstChild(name, namespaceUri);
      if(child != null)
      {
         rval = child.getValue();
      }
      
      return rval;
   }
   
   /**
    * Returns true if this element has a child with the specified name.
    * 
    * @param name the name of the child to check for.
    * 
    * @return true if this element has a child element with the given name,
    *         false if not.
    */
   public boolean hasChild(String name)
   {
      return getFirstChild(name) != null;
   }
   
   /**
    * Returns true if this element has a child with the specified name and
    * namespace URI.
    * 
    * @param name the name of the child to check for.
    * @param namespaceUri the namespace URI for the child to check for.
    * 
    * @return true if this element has a child element with the given name and
    *         namespace URI, false if not.
    */
   public boolean hasChild(String name, String namespaceUri)
   {
      return getFirstChild(name, namespaceUri) != null;
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
    * Gets the children for this XmlElement with the specified namespace URI.
    * Any changes to the children in the returned collection will be reflected
    * in this XmlElement.
    * 
    * @param namespaceUri the URI namespace of the children to return.
    * 
    * @return the children with the specified namespace URI for this XmlElement
    *         in a collection of XmlElements.
    */
   public Collection getChildrenWithNamespaceUri(String namespaceUri)
   {
      Vector rval = new Vector();
      
      for(Iterator i = mChildren.iterator(); i.hasNext();)
      {
         XmlElement child = (XmlElement)i.next();
         String childNamespaceUri = child.getNamespaceUri();
         if((childNamespaceUri == null && namespaceUri == null) ||
            (childNamespaceUri != null &&
             childNamespaceUri.equals(namespaceUri)))
         {
            rval.add(child);
         }
      }
      
      return rval;
   }
      
   /**
    * Gets the children for this XmlElement with the specified name and
    * namespace URI. Any changes to the children in the returned collection
    * will be reflected in this XmlElement.
    * 
    * @param name the name of the children to return.
    * @param namespaceUri the namespace URI of the children to return.
    * 
    * @return the children with the specified name and namespaceURI for this
    *         XmlElement in a collection of XmlElements.
    */
   public Collection getChildren(String name, String namespaceUri)
   {
      Vector rval = new Vector();
      
      for(Iterator i = mChildren.iterator(); i.hasNext();)
      {
         XmlElement child = (XmlElement)i.next();
         if(child.getName().equals(name))
         {
            String childNamespaceUri = child.getNamespaceUri();
            if((childNamespaceUri == null && namespaceUri == null) ||
               (childNamespaceUri != null &&
                childNamespaceUri.equals(namespaceUri)))
            {
               rval.add(child);
            }
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
    * Set to true to inherit this XmlElement's namespace URI from its parent
    * XmlElement if the namespace URI is set to null, false not to.
    * 
    * @param inherit true to inherit this XmlElement's namespace URI from
    *                its parent XmlElement if the namespace URI is set to
    *                null, false not to.
    */
   public void setInheritNamespaceUri(boolean inherit)
   {
      mInheritNamespaceUri = inherit;
   }
   
   /**
    * Gets whether to inherit this XmlElement's namespace URI from its parent
    * XmlElement if the namespace URI is set to null.
    * 
    * @return true to inherit this XmlElement's namespace URI from
    *         its parent XmlElement if the namespace URI is set to
    *         null, false not to.
    */
   public boolean getInheritNamespaceUri()   
   {
      return mInheritNamespaceUri;
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
   public static String parseNamespacePrefix(String name)
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
    * Gets the local name for the given fully qualified name (strips off
    * the namespace prefix, if any). 
    *
    * @param qName the qualified name to get the local name for.
    * 
    * @return the local name.
    */
   public static String parseLocalName(String qName)
   {
      String rval = qName;
      
      // split the name to get the namespace prefix, if any
      int index = qName.indexOf(":");
      if(index != -1)
      {
         // get the local name
         rval = qName.substring(index + 1);
      }
      
      return rval;
   }
}
