/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.wsdl;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.soap.WsdlSoapBinding;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;

/**
 * A WSDL is a Web Services Description Language. An instance of this class
 * represents a description for a particular web service.
 * 
 * FIXME:
 * Note: Namespaces are not handled quite right in the current implementation.
 * This needs to be corrected. Port type names, message names, etc -- should
 * all be checked against the target namespace. 
 * 
 * FIXME:
 * Note: The current implementation of this class assumes bindings converted
 * from xml will be SOAP bindings. This will be changed in the future so
 * classes that use this API should expect some minor changes.
 * 
 * This particular implementation of WSDL uses the WSDL schema provided here:
 * 
 * "http://schemas.xmlsoap.org/wsdl/"
 * 
 * It uses the HTTP SOAP schema (for soap bindings) provided here:
 * 
 * "http://schemas.xmlsoap.org/soap/http"
 * 
 * @author Dave Longley
 */
public class Wsdl extends AbstractXmlSerializer
{
   /**
    * The name for the web service. 
    */
   protected String mName;
   
   /**
    * The target namespace URI for the web service. 
    */
   protected String mTargetNamespaceUri;
   
   /**
    * The messages the web service understands.
    */
   protected WsdlMessageCollection mMessageCollection;
   
   /**
    * The port types for the web service that define sets of operations
    * the web service can perform.
    */
   protected WsdlPortTypeCollection mPortTypeCollection;
   
   /**
    * The bindings for the web service that provide the implementations
    * (according to some protocol -- currently only SOAP over HTTP is
    * supported) for the port types.
    */
   protected WsdlBindingCollection mBindingCollection;
   
   /**
    * The services for the web service that describe the ports that
    * provide the services for the web service.
    */
   protected WsdlServiceCollection mServiceCollection;   
   
   /**
    * A mapping from XML Schema data type to java class.
    */
   protected static HashMap<String, Class> smXMLSchemaDataTypeToClass =
      new HashMap<String, Class>();
   
   static
   {
      // supported XML Schema types -> java classes
      smXMLSchemaDataTypeToClass.put("xsd:string", String.class);
      smXMLSchemaDataTypeToClass.put("xsd:boolean", boolean.class);
      smXMLSchemaDataTypeToClass.put("xsd:byte", byte.class);
      smXMLSchemaDataTypeToClass.put("xsd:char", char.class);
      smXMLSchemaDataTypeToClass.put("xsd:short", short.class);
      smXMLSchemaDataTypeToClass.put("xsd:int", int.class);
      smXMLSchemaDataTypeToClass.put("xsd:long", long.class);
      smXMLSchemaDataTypeToClass.put("xsd:float", float.class);
      smXMLSchemaDataTypeToClass.put("xsd:double", double.class);
   }
   
   /**
    * A mapping from java class to XML Schema data type.
    */
   protected static HashMap<Class, String> smClassToXMLSchemaDataType =
      new HashMap<Class, String>();
   
   static
   {
      // supported java classes -> XML Schema types
      smClassToXMLSchemaDataType.put(String.class, "xsd:string");
      smClassToXMLSchemaDataType.put(boolean.class, "xsd:boolean");
      smClassToXMLSchemaDataType.put(Boolean.class, "xsd:boolean");
      smClassToXMLSchemaDataType.put(byte.class, "xsd:byte");
      smClassToXMLSchemaDataType.put(Byte.class, "xsd:byte");
      smClassToXMLSchemaDataType.put(char.class, "xsd:char");
      smClassToXMLSchemaDataType.put(Character.class, "xsd:char");
      smClassToXMLSchemaDataType.put(short.class, "xsd:short");
      smClassToXMLSchemaDataType.put(Short.class, "xsd:short");
      smClassToXMLSchemaDataType.put(int.class, "xsd:int");
      smClassToXMLSchemaDataType.put(Integer.class, "xsd:int");
      smClassToXMLSchemaDataType.put(long.class, "xsd:long");
      smClassToXMLSchemaDataType.put(Long.class, "xsd:long");
      smClassToXMLSchemaDataType.put(float.class, "xsd:float");
      smClassToXMLSchemaDataType.put(Float.class, "xsd:float");
      smClassToXMLSchemaDataType.put(double.class, "xsd:double");
      smClassToXMLSchemaDataType.put(Double.class, "xsd:double");      
   }
   
   /**
    * The XML Schema -- provides data types.
    */
   public static final String XML_SCHEMA_NAMESPACE_URI =
      "http://www.w3.org/2001/XMLSchema";
   
   /**
    * The WSDL namespace.
    */
   public static final String WSDL_NAMESPACE_URI =
      "http://schemas.xmlsoap.org/wsdl/";
   
   /**
    * The WSDL SOAP namespace.
    */
   public static final String WSDL_SOAP_NAMESPACE_URI =
      "http://schemas.xmlsoap.org/wsdl/soap/";   
   
   /**
    * Creates a new blank WSDL.
    */
   public Wsdl()
   {
      // set blank name and target namespace URI
      setName("");
      setTargetNamespaceUri("");
   }
   
   /**
    * Creates a new WSDL from the given port type interface.
    * 
    * @param name the name of the web service.
    * @param namespaceUri the namespaceUri for the web service.
    * @param portTypeInterface an interface for a port type.
    * @param bindingFactory the binding factory to use to generate a binding.
    * @param portFactory the port factory to use to generate a port.
    */
   public Wsdl(
      String name, String namespaceUri, Class portTypeInterface,
      WsdlBindingFactory bindingFactory, WsdlPortFactory portFactory)
   {
      // set name and namespace
      setName(name);
      setTargetNamespaceUri(namespaceUri);      
      
      // generate WSDL
      generateWsdl(portTypeInterface, bindingFactory, portFactory);
   }
   
   /**
    * Generates this WSDL from the given port type interface.
    * 
    * @param portTypeInterface an interface for a port type.
    * @param bindingFactory the binding factory to use to generate a binding.
    * @param portFactory the port factory to use to generate a port.
    */
   protected void generateWsdl(
      Class portTypeInterface,
      WsdlBindingFactory bindingFactory, WsdlPortFactory portFactory)
   {
      // create a new port type
      WsdlPortType portType =
         new WsdlPortType(this, portTypeInterface.getSimpleName());
      
      // use all of the declared methods in the port type interface
      Method[] methods = portTypeInterface.getDeclaredMethods();
      for(int i = 0; i < methods.length; i++)
      {
         Method m = methods[i];
         
         // create messages for the method
         WsdlMessage requestMessage =
            new WsdlMessage(this, m.getName());
         WsdlMessage responseMessage =
            new WsdlMessage(this, m.getName() + "Response");
         
         // go through all of the parameters for the method
         // build parameter order string
         String parameterOrder = "";
         Class[] types = m.getParameterTypes();
         for(int t = 0; t < types.length; t++)
         {
            // create message part
            WsdlMessagePart part = new WsdlMessagePart(
               types[t].getSimpleName() + "_" + (t + 1),
               Wsdl.getXsdDataType(types[t]));
            
            // add message part to request message
            requestMessage.getParts().add(part);

            // update parameter order
            if(parameterOrder.length() > 0)
            {
               parameterOrder += ",";
            }

            parameterOrder += part.getName();
         }
         
         // add the request message
         getMessages().add(requestMessage);
         
         // set the return type for the response message
         String returnType = Wsdl.getXsdDataType(m.getReturnType());
         if(returnType != null)
         {
            WsdlMessagePart part = new WsdlMessagePart("result", returnType);
            
            // add the message part to the response message
            responseMessage.getParts().add(part);
            
            // add the response message
            getMessages().add(responseMessage);
         }
         
         // create the port type operation
         WsdlPortTypeOperation operation =
            new WsdlPortTypeOperation(portType, m.getName());
         operation.setParameterOrder(parameterOrder);
         operation.setInputMessageName(requestMessage.getName());
         operation.setOutputMessageName(responseMessage.getName());
         operation.setInputFirst(true);
         
         // add the port type operation
         portType.getOperations().add(operation);
      }
      
      // add the port type
      getPortTypes().add(portType);
      
      // create and add a binding
      WsdlBinding binding =
         bindingFactory.createWsdlBinding(this, portType);
      getBindings().add(binding);
      
      // create a service
      WsdlService service = new WsdlService(this, getName());
      
      // create a port and add it to the service
      WsdlPort port = portFactory.createWsdlPort(this, binding);
      service.getPorts().add(port);
      
      // add the service
      getServices().add(service);
   }
   
   /**
    * Converts an XmlElement to its appropriate type. Throws an exception if
    * the XmlElement could not be converted.
    * 
    * @param value the XmlElement to convert.
    * @param type the type (as according to the definitions laid out in
    *             this Wsdl) to convert the value to.
    * 
    * @return the object the string was converted into.
    */
   public Object parseObject(XmlElement value, String type)   
   {
      // FUTURE CODE: add support for complex object types defined
      // in the Wsdl
      throw new UnsupportedOperationException(
         "This method has not been implemented yet.");
   }
   
   /**
    * Sets the name for the web service.
    * 
    * @param name the name for the web service.
    */
   public void setName(String name)
   {
      mName = name;
   }
   
   /**
    * Gets the name for the web service.
    * 
    * @return the name for the web service.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Sets the target namespace URI for the web service.
    * 
    * @param namespaceUri the target namespace URI for the web service.
    */
   public void setTargetNamespaceUri(String namespaceUri)
   {
      mTargetNamespaceUri = namespaceUri;
   }
   
   /**
    * Gets the target namespace URI for the web service.
    * 
    * @return the target namespace URI for the web service.
    */
   public String getTargetNamespaceUri()
   {
      return mTargetNamespaceUri;
   }
   
   /**
    * Gets the message definitions for the web service.
    * 
    * @return the message definitions for the web service.
    */
   public WsdlMessageCollection getMessages()
   {
      if(mMessageCollection == null)
      {
         // create message collection
         mMessageCollection = new WsdlMessageCollection();
      }
      
      return mMessageCollection;
   }

   /**
    * Gets the port type definitions for the web service.
    * 
    * @return the port type definitions for the web service.
    */
   public WsdlPortTypeCollection getPortTypes()
   {
      if(mPortTypeCollection == null)
      {
         // create port type collection
         mPortTypeCollection = new WsdlPortTypeCollection();
      }
      
      return mPortTypeCollection;
   }
   
   /**
    * Gets the binding definitions for the web service.
    * 
    * @return the binding definitions for the web service.
    */
   public WsdlBindingCollection getBindings()
   {
      if(mBindingCollection == null)
      {
         // create binding collection
         mBindingCollection = new WsdlBindingCollection();
      }
      
      return mBindingCollection;
   }
   
   /**
    * Gets the service definitions for the web service.
    * 
    * @return the service definitions for the web service.
    */
   public WsdlServiceCollection getServices()
   {
      if(mServiceCollection == null)
      {
         // create service collection
         mServiceCollection = new WsdlServiceCollection();
      }
      
      return mServiceCollection;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "definitions";
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
      // create xml element
      XmlElement element = new XmlElement(getRootTag());
      element.setParent(parent);
      
      // add attributes
      element.addAttribute("xmlns", WSDL_NAMESPACE_URI);
      element.addAttribute("xmlns:xsd", XML_SCHEMA_NAMESPACE_URI);
      element.addAttribute("xmlns:soap", WSDL_SOAP_NAMESPACE_URI);
      element.addAttribute("xmlns:tns", getTargetNamespaceUri());
      element.addAttribute("name", getName());
      element.addAttribute("targetNamespace", getTargetNamespaceUri());
      
      // FUTURE CODE: add code for creating custom types,
      // current implementation only supports primitive data types 
      element.addChild(new XmlElement("types"));
      
      // messages
      for(WsdlMessage message: getMessages())
      {
         element.addChild(message.convertToXmlElement(element));
      }

      // port types
      for(WsdlPortType portType: getPortTypes())
      {
         element.addChild(portType.convertToXmlElement(element));
      }
      
      // bindings
      for(WsdlBinding binding: getBindings())
      {
         element.addChild(binding.convertToXmlElement(element));
      }
      
      // services
      for(WsdlService service: getServices())
      {
         element.addChild(service.convertToXmlElement(element));
      }
      
      return element;
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
      boolean rval = false;
      
      if(element.getName().equals(getRootTag()))
      {
         rval = true;
         
         // set blank name and target namespace URI
         setName("");
         setTargetNamespaceUri("");
         
         // clear collections
         getMessages().clear();
         getPortTypes().clear();
         getBindings().clear();
         getServices().clear();
         
         // get name
         setName(element.getAttributeValue("name"));
         
         // get target namespace URI
         setTargetNamespaceUri(element.getAttributeValue("targetNamespace"));
         
         // convert messages
         for(XmlElement child: element.getChildren("message"))
         {
            WsdlMessage message = new WsdlMessage(this);
            if(message.convertFromXmlElement(child))
            {
               getMessages().add(message);
            }
         }
         
         // convert port types
         for(XmlElement child: element.getChildren("portType"))
         {
            WsdlPortType portType = new WsdlPortType(this);
            if(portType.convertFromXmlElement(child))
            {
               getPortTypes().add(portType);
            }
         }
         
         // convert bindings
         for(XmlElement child: element.getChildren("binding"))
         {
            // FUTURE CODE: current implementation assumes a soap binding
            WsdlBinding binding = new WsdlSoapBinding(this);
            if(binding.convertFromXmlElement(child))
            {
               getBindings().add(binding);
            }
         }
         
         // convert services
         for(XmlElement child: element.getChildren("service"))
         {
            WsdlService service = new WsdlService(this);
            if(service.convertFromXmlElement(child))
            {
               getServices().add(service);
            }
         }
      }
      
      return rval;
   }   
   
   /**
    * This method takes XML text (in full document form) and converts
    * it to it's internal representation.
    *
    * @param xmlText the xml text document that represents the object.
    * 
    * @return true if successful, false otherwise.    
    */
   public boolean convertFromXml(String xmlText)
   {
      boolean rval = false;
      
      getLogger().debug(getClass(), "Loading WSDL from xml...");
      getLogger().debugData(getClass(), "WSDL:\n" + xmlText);
      
      rval = super.convertFromXml(xmlText);
      
      return rval;
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
   
   /**
    * Gets an xsd data type from the given java class.
    * 
    * @param type the java class.
    * 
    * @return the xsd data type.
    */
   public static String getXsdDataType(Class type)
   {
      return smClassToXMLSchemaDataType.get(type); 
   }
   
   /**
    * Gets a class from the given xsd data type.
    * 
    * @param xsdType the xsd data type.
    * 
    * @return type the java class.
    */
   public static Class getJavaClass(String xsdType)
   {
      return smXMLSchemaDataTypeToClass.get(xsdType); 
   }
   
   /**
    * Converts a string to its appropriate type. Throws an exception if
    * the string could not be converted.
    * 
    * @param value the string to convert.
    * @param xsdType the xsd type to convert the string to.
    * 
    * @return the object the string was converted into.
    */
   public static Object parseObject(String value, String xsdType)   
   {
      return parseObject(value, getJavaClass(xsdType));
   }
   
   /**
    * Converts a string to its appropriate type. Throws an exception if
    * the string could not be converted.
    * 
    * @param value the string to convert.
    * @param type the type to convert the string to.
    * 
    * @return the object the string was converted into.
    */
   public static Object parseObject(String value, Class type)
   {
      Object rval = null;
      
      if(value != null)
      {
         if(type == String.class)
         {
            rval = value;
         }
         else if(type == boolean.class || type == Boolean.class)
         {
            boolean b = Boolean.parseBoolean(value);
            rval = new Boolean(b);
         }
         else if(type == byte.class || type == Byte.class)
         {
            byte b = value.getBytes()[0];
            rval = new Byte(b);
         }
         else if(type == char.class || type == Character.class)
         {
            char c = value.charAt(0);
            rval = new Character(c);
         }
         else if(type == short.class || type == Short.class)
         {
            short s = Short.parseShort(value);
            rval = new Short(s);
         }
         else if(type == int.class || type == Integer.class)
         {
            int i = Integer.parseInt(value);
            rval = new Integer(i);
         }
         else if(type == long.class || type == Long.class)
         {
            long l = Long.parseLong(value);
            rval = new Long(l);
         }
         else if(type == float.class || type == Float.class)
         {
            float f = Float.parseFloat(value);
            rval = new Float(f);
         }
         else if(type == double.class || type == Double.class)
         {
            double d = Double.parseDouble(value);
            rval = new Double(d);
         }
      }

      return rval;
   }
   
   /**
    * A WSDL Message collection.
    * 
    * @author Dave Longley
    */
   public class WsdlMessageCollection implements Iterable<WsdlMessage>
   {
      /**
       * The underlying vector for storing messages. 
       */
      protected Vector<WsdlMessage> mMessages;
      
      /**
       * Creates a new WsdlMessageCollection.
       */
      public WsdlMessageCollection()
      {
         // initialize messages vector
         mMessages = new Vector<WsdlMessage>();
      }
      
      /**
       * Adds a new message to this collection.
       * 
       * @param message the message to add to this collection.
       */
      public void add(WsdlMessage message)
      {
         mMessages.add(message);
      }
      
      /**
       * Removes a message from this collection.
       * 
       * @param message the message to remove from this collection.
       */
      public void remove(WsdlMessage message)
      {
         mMessages.remove(message);
      }
      
      /**
       * Gets a message from this collection according to its name.
       * 
       * @param name the name of the message to retrieve.
       * 
       * @return the message or null if one was not found.
       */
      public WsdlMessage getMessage(String name)
      {
         WsdlMessage rval = null;
         
         // FIXME: we need to check namespaces better
         // strip off the namespace prefix
         name = XmlElement.parseLocalName(name);
         
         for(Iterator<WsdlMessage> i = iterator(); i.hasNext() && rval == null;) 
         {
            WsdlMessage message = i.next();
            if(message.getName().equals(name))
            {
               rval = message;
            }
         }
         
         if(rval == null)
         {
            getLogger().warning(getClass(),
               "WsdlMessage not found,name=" + name);
         }
         
         return rval;
      }
      
      /**
       * Clears all the messages from this collection.
       */
      public void clear()
      {
         mMessages.clear();
      }
      
      /**
       * Gets an iterator over the messages in this collection.
       * 
       * @return an iterator over the messages in this collection.
       */
      public Iterator<WsdlMessage> iterator()
      {
         return mMessages.iterator();
      }
      
      /**
       * Gets the number of messages in this collection.
       * 
       * @return the number of messages in this collection.
       */
      public int size()
      {
         return mMessages.size();
      }
   }
   
   /**
    * A WSDL Port Type collection.
    * 
    * @author Dave Longley
    */
   public class WsdlPortTypeCollection implements Iterable<WsdlPortType>
   {
      /**
       * The underlying vector for storing port types. 
       */
      protected Vector<WsdlPortType> mPortTypes;
      
      /**
       * Creates a new WsdlPortTypeCollection.
       */
      public WsdlPortTypeCollection()
      {
         // initialize port types vector
         mPortTypes = new Vector<WsdlPortType>();
      }
      
      /**
       * Adds a new port type to this collection.
       * 
       * @param portType the port type to add to this collection.
       */
      public void add(WsdlPortType portType)
      {
         mPortTypes.add(portType);
      }
      
      /**
       * Removes a port type from this collection.
       * 
       * @param portType the port type to remove from this collection.
       */
      public void remove(WsdlPortType portType)
      {
         mPortTypes.remove(portType);
      }
      
      /**
       * Gets a port type from this collection according to its name.
       * 
       * @param name the name of the port type to retrieve.
       * 
       * @return the port type or null if one was not found.
       */
      public WsdlPortType getPortType(String name)
      {
         WsdlPortType rval = null;
         
         // FIXME: we need to check namespaces better
         // strip off the namespace prefix
         name = XmlElement.parseLocalName(name);
         
         for(Iterator<WsdlPortType> i = iterator(); i.hasNext() && rval == null;) 
         {
            WsdlPortType portType = i.next();
            if(portType.getName().equals(name))
            {
               rval = portType;
            }
         }
         
         if(rval == null)
         {
            getLogger().warning(getClass(), "PortType not found,name=" + name);
         }
         
         return rval;
      }      
      
      /**
       * Clears all the port types from this collection.
       */
      public void clear()
      {
         mPortTypes.clear();
      }
      
      /**
       * Gets an iterator over the port types in this collection.
       * 
       * @return an iterator over the port types in this collection.
       */
      public Iterator<WsdlPortType> iterator()
      {
         return mPortTypes.iterator();
      }
      
      /**
       * Gets the number of port types in this collection.
       * 
       * @return the number of port types in this collection.
       */
      public int size()
      {
         return mPortTypes.size();
      }
   }
   
   /**
    * A WSDL Binding collection.
    * 
    * @author Dave Longley
    */
   public class WsdlBindingCollection implements Iterable<WsdlBinding>
   {
      /**
       * The underlying vector for storing bindings. 
       */
      protected Vector<WsdlBinding> mBindings;
      
      /**
       * Creates a new WsdlBindingCollection.
       */
      public WsdlBindingCollection()
      {
         // initialize bindings vector
         mBindings = new Vector<WsdlBinding>();
      }
      
      /**
       * Adds a new bindings to this collection.
       * 
       * @param binding the bindings to add to this collection.
       */
      public void add(WsdlBinding binding)
      {
         mBindings.add(binding);
      }
      
      /**
       * Removes a bindings from this collection.
       * 
       * @param binding the bindings to remove from this collection.
       */
      public void remove(WsdlBinding binding)
      {
         mBindings.remove(binding);
      }
      
      /**
       * Gets a binding from this collection according to its name.
       * 
       * @param name the name of the binding to retrieve.
       * 
       * @return the binding or null if one was not found.
       */
      public WsdlBinding getBinding(String name)
      {
         WsdlBinding rval = null;
         
         // FIXME: we need to check namespaces better
         // strip off the namespace prefix
         name = XmlElement.parseLocalName(name);
         
         for(Iterator<WsdlBinding> i = iterator(); i.hasNext() && rval == null;) 
         {
            WsdlBinding binding = i.next();
            if(binding.getName().equals(name))
            {
               rval = binding;
            }
         }
         
         if(rval == null)
         {
            getLogger().warning(getClass(), "Binding not found,name=" + name);
         }         
         
         return rval;
      }      
      
      /**
       * Clears all the bindings from this collection.
       */
      public void clear()
      {
         mBindings.clear();
      }
      
      /**
       * Gets an iterator over the bindings in this collection.
       * 
       * @return an iterator over the bindings in this collection.
       */
      public Iterator<WsdlBinding> iterator()
      {
         return mBindings.iterator();
      }

      /**
       * Gets the number of bindings in this collection.
       * 
       * @return the number of bindings in this collection.
       */
      public int size()
      {
         return mBindings.size();
      }
   }
   
   /**
    * A WSDL Service collection.
    * 
    * @author Dave Longley
    */
   public class WsdlServiceCollection implements Iterable<WsdlService>
   {
      /**
       * The underlying vector for storing services. 
       */
      protected Vector<WsdlService> mServices;
      
      /**
       * Creates a new WsdlServiceCollection.
       */
      public WsdlServiceCollection()
      {
         // initialize services vector
         mServices = new Vector<WsdlService>();
      }
      
      /**
       * Adds a new services to this collection.
       * 
       * @param service the services to add to this collection.
       */
      public void add(WsdlService service)
      {
         mServices.add(service);
      }
      
      /**
       * Removes a services from this collection.
       * 
       * @param service the services to remove from this collection.
       */
      public void remove(WsdlService service)
      {
         mServices.remove(service);
      }
      
      /**
       * Gets a service from this collection according to its name.
       * 
       * @param name the name of the service to retrieve.
       * 
       * @return the service or null if one was not found.
       */
      public WsdlService getService(String name)
      {
         WsdlService rval = null;
         
         // FIXME: we need to check namespaces better
         // strip off the namespace prefix
         name = XmlElement.parseLocalName(name);
         
         for(Iterator<WsdlService> i = iterator(); i.hasNext() && rval == null;) 
         {
            WsdlService service = i.next();
            if(service.getName().equals(name))
            {
               rval = service;
            }
         }
         
         if(rval == null)
         {
            getLogger().warning(getClass(), "Service not found,name=" + name);
         }         
         
         return rval;
      }      
      
      /**
       * Clears all the services from this collection.
       */
      public void clear()
      {
         mServices.clear();
      }
      
      /**
       * Gets an iterator over the services in this collection.
       * 
       * @return an iterator over the services in this collection.
       */
      public Iterator<WsdlService> iterator()
      {
         return mServices.iterator();
      }
      
      /**
       * Gets the number of services in this collection.
       * 
       * @return the number of services in this collection.
       */
      public int size()
      {
         return mServices.size();
      }
   }   
}
