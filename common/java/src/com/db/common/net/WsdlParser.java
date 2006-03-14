/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.net;

import com.db.common.ElementReader;
import com.db.common.IXMLSerializer;
import com.db.common.logging.Logger;
import com.db.common.logging.LoggerManager;

import java.io.StringReader;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

/**
 * This class parses a WSDL for a soap client.
 * 
 * @author Dave Longley
 */
public class WsdlParser implements IXMLSerializer
{
   /**
    * Data members that describe the web service.
    */
   protected String mName;
   protected String mNamespace;
   protected String mPortType;
   
   /**
    * A map of method name to param order.
    */
   protected HashMap mMethodToParamOrder;
   
   /**
    * A map of message to return type.
    */
   protected HashMap mMessageToReturnType;
   
   /**
    * A mapping from xsd type to class.
    */
   protected HashMap mXsdTypeToClass;
   
   /**
    * Creates a new WSDL parser.
    */
   public WsdlParser()
   {
      reset();
      
      mXsdTypeToClass = new HashMap();
      
      // supported xsd types
      mXsdTypeToClass.put("xsd:string", String.class);
      mXsdTypeToClass.put("xsd:boolean", boolean.class);
      mXsdTypeToClass.put("xsd:byte", byte.class);
      mXsdTypeToClass.put("xsd:char", char.class);
      mXsdTypeToClass.put("xsd:short", short.class);
      mXsdTypeToClass.put("xsd:int", int.class);
      mXsdTypeToClass.put("xsd:long", long.class);
      mXsdTypeToClass.put("xsd:float", float.class);
      mXsdTypeToClass.put("xsd:double", double.class);
   }
   
   /**
    * Resets data members.
    */
   protected void reset()
   {
      mName = "";
      mNamespace = "";
      mPortType = "";
      
      mMethodToParamOrder = new HashMap();
      mMessageToReturnType = new HashMap();
   }
   
   /**
    * Builds a param hashmap out of the vector of params provided.
    * 
    * @param method the name of the method the params are for.
    * @param params a vector of ordered params.
    * @return a map of params (name -> value) for a remote method, or null
    *         if an error occurred (like wrong number of params).
    */
   protected HashMap getParamMap(String method, Vector params)
   {
      HashMap paramMap = null;
      
      // get the param order
      String paramOrder = (String)mMethodToParamOrder.get(method);
      if(paramOrder != null)
      {
         String[] split = paramOrder.split(",");
         if(params.size() == split.length)
         {
            paramMap = new HashMap();
            
            // associate parameter name with parameter
            for(int i = 0; i < split.length; i++)
            {
               paramMap.put(split[i], params.get(i));
            }
         }
         else if(params.size() == 0 && split.length == 1 &&
                 split[0].equals(""))
         {
            // hashmap is empty, no params for method
            paramMap = new HashMap();
         }
         else
         {
            getLogger().error("Wrong num of params for method: " + method);
         }
      }
      else
      {
         getLogger().error("no such method exists!: " + method);
      }
      
      return paramMap;
   }
   
   /**
    * This method takes options that are used to configure
    * how to convert to and from xml.
    *
    * @param options the configuration options.
    * @return true if options successfully set, false if not.    
    */
   public boolean setSerializerOptions(int options)
   {
      return false;
   }

   /**
    * This method gets the options that are used to configure
    * how to convert to and from xml.
    *
    * @return the configuration options.
    */
   public int getSerializerOptions()
   {
      return 0;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "";
   }   

   /**
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    * 
    * @return the xml-based representation of the object.
    */
   public String convertToXML()
   {
      return convertToXML(0);
   }
   
   /**
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    *
    * @param indentLevel the number of spaces to place before the text
    *                    after each new line.
    * @return the xml-based representation of the object.
    */
   public String convertToXML(int indentLevel)
   {
      return "";
   }
   
   /**
    * This method takes XML text (in full document form) and converts
    * it to it's internal representation.
    *
    * @param xmlText the xml text document that represents the object.
    * @return true if successful, false otherwise.    
    */
   public boolean convertFromXML(String xmlText)
   {
      boolean rval = false;
      
      getLogger().debug("WSDL:\n" + xmlText);
      
      getLogger().debug("Loading soap message from xml...");

      try
      {
         DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
         DocumentBuilder builder = factory.newDocumentBuilder();
         
         InputSource is = new InputSource(new StringReader(xmlText));
         Document doc = builder.parse(is);
         
         // normalize text representation
         doc.getDocumentElement().normalize();
         
         rval = convertFromXML(doc.getDocumentElement());
      }
      catch(SAXParseException spe)
      {
         getLogger().debug("WsdlParser parsing error"
                            + ", line " + spe.getLineNumber()
                            + ", uri " + spe.getSystemId());
         getLogger().debug("   " + spe.getMessage());
         // print stack trace as below
      }
      catch(SAXException se)
      {
         Exception x = se.getException();
         x = (x == null) ? se : x;
         
         getLogger().debug(Logger.getStackTrace(x));
      }
      catch(Exception e)
      {
         getLogger().debug(Logger.getStackTrace(e));
      }    
      
      return rval;
   }
   
   /**
    * This method takes a parsed DOM XML element and converts it
    * back into this object's representation.
    *
    * @param element the parsed element that contains this objects information.
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXML(Element element)
   {
      boolean rval = false;
      
      // we need to convert according to schema to follow standards and
      // ensure compatibility
      
      // reset members
      reset();

      ElementReader er = new ElementReader(element);
      
      // get web service name
      setName(er.getStringAttribute("name"));
      
      // get web service namespace
      setNamespace(er.getStringAttribute("targetNamespace"));
      
      // get method return types
      Iterator i = er.getElementReaders("message").iterator();
      while(i.hasNext())
      {
         ElementReader reader = (ElementReader)i.next();

         String messageName = reader.getStringAttribute("name");
         if(messageName.endsWith("Response"))
         {
            ElementReader part = reader.getFirstElementReader("part");
            if(part != null)
            {
               String resultType = part.getStringAttribute("type");
               
               // add mapping
               mMessageToReturnType.put(messageName, resultType);
            }
         }
      }
      
      // get port type
      er = er.getFirstElementReader("portType");
      if(er != null)
      {
         setPortType(er.getStringAttribute("name"));
         
         // get method param orders
         i = er.getElementReaders("operation").iterator();
         while(i.hasNext())
         {
            ElementReader reader = (ElementReader)i.next();
            
            String methodName = reader.getStringAttribute("name");
            String paramOrder = reader.getStringAttribute("parameterOrder");
               
            // add mapping
            mMethodToParamOrder.put(methodName, paramOrder);
         }
         
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Parses a WSDL.
    * 
    * @param wsdl the WSDL to parse.
    * @return true if successfully parsed, false if not.
    */
   public boolean parseWsdl(String wsdl)
   {
      return convertFromXML(wsdl);
   }
   
   /**
    * Gets the return type for a method.
    * 
    * @param method the method to get the return type for.
    * @return the return type for the method.
    */
   public Class getReturnType(String method)
   {
      Class rval = null;
      
      String messageName = getPortType() + "_" + method + "Response";
      String resultType = (String)mMessageToReturnType.get(messageName);
      rval = (Class)mXsdTypeToClass.get(resultType);
      
      return rval;
   }
   
   /**
    * Converts a string to its appropriate type. Throws an exception if
    * the string could not be converted.
    * 
    * @param value the string to convert.
    * @param type the type to convert the string to.
    * @return the object the string was converted into.
    * @throws Exception
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
    * Sets the namespace for the web service.
    * 
    * @param namespace the namespace for the web service.
    */
   public void setNamespace(String namespace)
   {
      mNamespace = namespace;
   }
   
   /**
    * Gets the namespace for the web service.
    * 
    * @return the namespace for the web service.
    */
   public String getNamespace()
   {
      return mNamespace;
   }
   
   /**
    * Sets the port type for the web service.
    * 
    * @param portType the port type for the web service.
    */
   public void setPortType(String portType)
   {
      mPortType = portType;
   }
   
   /**
    * Gets the port type for the web service.
    * 
    * @return the port type for the web service.
    */
   public String getPortType()
   {
      return mPortType;
   }   
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbcommon");
   }
}
