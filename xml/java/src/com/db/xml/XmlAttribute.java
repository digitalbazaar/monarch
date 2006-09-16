/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

/**
 * An XmlAttribute is an attribute for an XmlElement. It has a name and value.
 * 
 * @author Dave Longley
 */
public class XmlAttribute
{
   /**
    * The name for this XmlAttribute.
    */
   protected String mName;
   
   /**
    * The value for this XmlAttribute.
    */
   protected String mValue;
   
   /**
    * The namespace URI for this attribute. This is the URI that points to
    * the definition for this attribute.
    */
   protected String mNamespaceUri;
   
   /**
    * The XmlElement that is the parent of this attribute.
    */
   protected XmlElement mParent;
   
   /**
    * Creates a new blank XmlAttribute.
    */
   public XmlAttribute()
   {
      this("", "", null);
   }
   
   /**
    * Creates a new XmlAttribute with the specified name, value,
    * and namespace URI.
    * 
    * @param name the name of the XmlAttribute.
    * @param value the value of the XmlAttribute.
    * @param namespaceUri the namespace URI for this XmlAttribute.
    */
   public XmlAttribute(String name, String value, String namespaceUri)
   {
      setName(name);
      setValue(value);
      setNamespaceUri(namespaceUri);
   }
   
   /**
    * Sets the name for this XmlAttribute.
    * 
    * @param name the name for this XmlAttribute.
    */
   public void setName(String name)
   {
      mName = name;
   }
   
   /**
    * Gets the name for this XmlAttribute.
    * 
    * @return the name for this XmlAttribute.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Sets the value for this XmlAttribute.
    * 
    * @param value the value for this XmlAttribute.
    */
   public void setValue(String value)
   {
      if(value == null)
      {
         value = "";
      }
      
      mValue = value;
   }
   
   /**
    * Gets the value for this XmlAttribute.
    * 
    * @return the value for this XmlAttribute.
    */
   public String getValue()
   {
      return mValue;
   }
   
   /**
    * Sets the namespace URI of this XmlAttribute.
    * 
    * @param namespaceUri the URI that points to the definition for the
    *                     namespace.
    */
   public void setNamespaceUri(String namespaceUri)
   {
      mNamespaceUri = namespaceUri;
   }
   
   /**
    * Gets the namespace URI of this XmlAttribute.
    * 
    * @return the namespace URI of this XmlAttribute.
    */
   public String getNamespaceUri()
   {
      String rval = mNamespaceUri;
      
      // if this attribute has no set namespace URI and it is not
      // a reserved name, and it has a parent XmlElement, get the
      // namespace URI from that element
      if(mNamespaceUri == null &&
         !getValue().equals("xml") && !getValue().equals("xmlns") &&
         getParent() != null)
      {
         rval = getParent().getNamespaceUri();
      }
      
      return rval;
   }
   
   /**
    * Sets the XmlElement that is the parent of this XmlAttribute.
    * 
    * @param parent the XmlElement that is the parent of this XmlAttribute.
    */
   public void setParent(XmlElement parent)
   {
      mParent = parent;
   }
   
   /**
    * Gets the XmlElement that is the parent of this XmlAttribute.
    * 
    * @return the XmlElement that is the parent of this XmlAttribute.
    */
   public XmlElement getParent()
   {
      return mParent;
   }
   
   /**
    * Gets the qualified name of this XmlAttribute. If this XmlAttribute has
    * a different namespace URI from its parent XmlElement, then this includes
    * its namespace prefix, followed by a colon, followed by its local name,
    * otherwise only the local name (or a local name prefixed by "xmlns") will
    * be returned.
    * 
    * @return the qualified name of this XmlAttribute.
    */
   public String getQualifiedName()
   {
      String rval = getName();
      
      // compare the namespace URI for this attribute with its parent element
      String attributeNamespaceUri = getNamespaceUri();
      String parentNamespaceUri = getParent().getNamespaceUri();
      
      // if the attribute namespace URI is null, there is no prefix --
      // otherwise see if the attribute namespace URI is not equal to the
      // parent element's namespace URI
      if(attributeNamespaceUri != null &&
         !attributeNamespaceUri.equals(parentNamespaceUri))
      {
         String prefix = getParent().findNamespacePrefix(attributeNamespaceUri);
      
         if(prefix != null)
         {
            rval = prefix + ":" + getName();
         }
      }
      
      return rval;
   }
   
   /**
    * Determines if this attribute is equal to another attribute.
    * 
    * @param obj the attribute to check for equality against.
    * 
    * @return true if this attribute is equal to another attribute,
    *         false if not.
    */
   public boolean equals(Object obj)
   {
      boolean rval = false;
      
      if(this != obj)
      {
         if(obj instanceof XmlAttribute && obj != null)
         {
            XmlAttribute attribute = (XmlAttribute)obj;
            String name = attribute.getName();
            String value = attribute.getValue();
            String namespaceUri = attribute.getNamespaceUri();
            
            // check name, namespace, and value
            if((getName() == null && name == null) ||
               (getName() != null && getName().equals(name)))
            {
               String thisNamespaceUri = getNamespaceUri();
               
               if((thisNamespaceUri == null && namespaceUri == null) ||
                  (thisNamespaceUri != null &&
                   thisNamespaceUri.equals(namespaceUri)))
               {
                  if((getValue() == null && value == null) ||
                     (getValue() != null && getValue().equals(value)))
                  {
                     rval = true;
                  }
               }
            }
         }
      }
      else
      {
         rval = true;
      }
      
      return rval;
   }
}
