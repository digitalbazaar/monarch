/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

import java.lang.reflect.Method;

/**
 * An XmlSerializer is used to serialize objects to xml and deserialize
 * them from xml.
 * 
 * @author Dave Longley
 */
public class XmlSerializer
{
   /**
    * Creates a new XmlSerializer.
    */
   public XmlSerializer()
   {
   }
   
   /**
    * Serializes the passed Object to an XmlElement with no set parent element.
    * 
    * @param obj the object to serialize.
    * 
    * @return the XmlElement that represents the passed object.
    * 
    * @exception XmlSerializationException thrown if an error occurs during
    *            serialization.
    */
   public XmlElement serialize(Object obj)
   throws XmlSerializationException
   {
      // serialize with no parent element
      return serialize(obj, null);
   }
   
   /**
    * Serializes the passed Object to an XmlElement that has the passed
    * XmlElement as a parent.
    * 
    * @param obj the object to serialize.
    * @param parent the XmlElement that is to be the parent of the
    *               new XmlElement.
    * 
    * @return the XmlElement that represents the passed object.
    * 
    * @exception XmlSerializationException thrown if an error occurs during
    *            serialization.
    */
   public XmlElement serialize(Object obj, XmlElement parent)
   throws XmlSerializationException
   {
      XmlElement element = null;
      
      try
      {
         // get the serializable annotation for the class
         XmlSerializableAnnotation serializableAnnotation =
            obj.getClass().getAnnotation(XmlSerializableAnnotation.class);
         if(serializableAnnotation != null)
         {
            // create the xml element with the annotation root element name
            element = new XmlElement(serializableAnnotation.rootElementName());
            
            // set the element's parent element
            element.setParent(parent);
            
            // set whether or not the element inherits the parent's namespace URI
            element.setInheritNamespaceUri(
               serializableAnnotation.inheritNamespaceUri());
            
            // set the element's namespace URI, if present
            if(!serializableAnnotation.namespaceUri().equals(""))
            {
               element.setNamespaceUri(serializableAnnotation.namespaceUri());
            }
            
            // go through all of the object's public methods and
            // create elements and attributes
            for(Method method: obj.getClass().getMethods())
            {
               // see if the method return value is for an xml element
               if(method.isAnnotationPresent(XmlElementAnnotation.class))
               {
                  // get the element annotation
                  XmlElementAnnotation elementAnnotation =
                     method.getAnnotation(XmlElementAnnotation.class);
                  
                  // determine if a child element should be created or
                  // if the value should be set for the root element based
                  // on whether or not the element name and namespaces
                  // are identical
                  XmlElement current = null;
                  if(elementAnnotation.name().equals(element.getName()) &&
                     ((elementAnnotation.namespaceUri().equals("") &&
                       element.getNamespaceUri() == null) ||
                      (elementAnnotation.namespaceUri().equals(
                       element.getNamespaceUri()))))
                  {
                     // the name and namespaces are equal, so
                     // set the current element to the element
                     current = element;
                  }
                  else
                  {
                     // create a child xml element
                     XmlElement child = new XmlElement(
                        elementAnnotation.name());
                     
                     // add the child
                     element.addChild(child);
                     
                     // set the current element to the child
                     current = child;
                  }
                  
                  // set whether or not the current element should inherit its
                  // parent's namespace URI
                  current.setInheritNamespaceUri(
                     elementAnnotation.inheritNamespaceUri());
                  
                  // set the current element's namespace URI, if present
                  if(!elementAnnotation.namespaceUri().equals(""))
                  {
                     current.setNamespaceUri(elementAnnotation.namespaceUri());
                  }
                  
                  // get the return value from the method
                  Object value = method.invoke(obj);
                  
                  // determine how to serialize the return value
                  if(value instanceof Iterable &&
                     elementAnnotation.serializeIterableAsChildren())
                  {
                     // serialize the return value by iterating over it
                     // and serializing each element
                     if(value != null)
                     {
                        // serialize each object
                        for(Object o: (Iterable)value)
                        {
                           // serialize and add the result as a child to
                           // the child element
                           XmlElement e = serialize(o, current);
                           current.addChild(e);
                        }
                     }
                  }
                  else
                  {
                     // determine if the return value is serializable or
                     // if it should be set as the value of the element
                     if(value != null && value.getClass().isAnnotationPresent(
                           XmlSerializableAnnotation.class))
                     {
                        // serialize return value as a child to
                        // the child element
                        XmlElement e = serialize(value, current);
                        current.addChild(e);
                     }
                     else if(value == null &&
                             method.getReturnType().isAnnotationPresent(
                                XmlSerializableAnnotation.class))
                     {
                        // value is null, but return type indicates an
                        // xml element, so do nothing
                     }
                     else
                     {
                        // use the return value as the value of the child
                        // element
                        current.setValue(String.valueOf(value));
                     }
                  }
               }
               else if(method.isAnnotationPresent(XmlAttributeAnnotation.class))
               {
                  // get the attribute annotation
                  XmlAttributeAnnotation attributeAnnotation =
                     method.getAnnotation(XmlAttributeAnnotation.class);
                  
                  // get the namespace URI for the attribute
                  String namespaceUri = attributeAnnotation.namespaceUri();
                  if(namespaceUri.equals(""))
                  {
                     namespaceUri = null;
                  }
                  
                  // get the return value from the method
                  Object value = method.invoke(obj);
                  
                  // add the attribute to the element
                  element.addAttribute(
                     attributeAnnotation.name(), String.valueOf(value),
                     namespaceUri, attributeAnnotation.inheritNamespaceUri());
               }
            }
         }
         else
         {
            // object is not xml serializable
            throw new XmlSerializationException(
               "Object does not have annotation 'XmlSerializableAnnotation'!");
         }
      }
      catch(Throwable t)
      {
         // throw new XmlSerializationException
         throw new XmlSerializationException(
            "Could not serialize object to xml!,cause= " + t, t);
      }
      
      return element;
   }
   
   /**
    * Deserializes the passed XmlElement to an Object.
    * 
    * @param element the XmlElement to deserialize.
    * @param obj the object to deserialize to.
    */
   public void deserialize(XmlElement element, Object obj)
   {
      // FIXME:
   }
}
