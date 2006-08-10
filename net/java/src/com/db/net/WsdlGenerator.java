/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

import java.lang.reflect.Method;
import java.util.HashMap;

/**
 * This class generates a WSDL for a soap handler.
 * 
 * @author Dave Longley
 */
public class WsdlGenerator
{
   /**
    * The wsdl schema.
    */
   public static final String WSDL_NAMESPACE =
      "http://schemas.xmlsoap.org/wsdl/";
   
   /**
    * The http schema.
    */
   public static final String HTTP_SCHEMA =
      "http://schemas.xmlsoap.org/soap/http";
   
   /**
    * A mapping from class to xsd type.
    */
   protected HashMap mClassToXsdType;
   
   /**
    * Creates a new WSDL generator.
    */
   public WsdlGenerator()
   {
      mClassToXsdType = new HashMap();
      
      // supported xsd types
      mClassToXsdType.put(String.class, "xsd:string");
      mClassToXsdType.put(boolean.class, "xsd:boolean");
      mClassToXsdType.put(Boolean.class, "xsd:boolean");
      mClassToXsdType.put(byte.class, "xsd:byte");
      mClassToXsdType.put(Byte.class, "xsd:byte");
      mClassToXsdType.put(char.class, "xsd:char");
      mClassToXsdType.put(Character.class, "xsd:char");
      mClassToXsdType.put(short.class, "xsd:short");
      mClassToXsdType.put(Short.class, "xsd:short");
      mClassToXsdType.put(int.class, "xsd:int");
      mClassToXsdType.put(Integer.class, "xsd:int");
      mClassToXsdType.put(long.class, "xsd:long");
      mClassToXsdType.put(Long.class, "xsd:long");
      mClassToXsdType.put(float.class, "xsd:float");
      mClassToXsdType.put(Float.class, "xsd:float");
      mClassToXsdType.put(double.class, "xsd:double");
      mClassToXsdType.put(Double.class, "xsd:double");
   }
   
   /**
    * Generates xml for messages.
    * 
    * @param port the port type.
    * @param wsInterface the web service interface.
    * 
    * @return the xml for messages.
    */
   protected String generateMessagesXml(String port, Class wsInterface)
   {
      StringBuffer xml = new StringBuffer();
      
      Method[] methods = wsInterface.getDeclaredMethods();
      for(int i = 0; i < methods.length; i++)
      {
         Method m = methods[i];
         String name = m.getName();
         
         Class[] types = m.getParameterTypes();
         if(types.length == 0)
         {
            xml.append(" <message name=\"" + port + "_" + name + "\"/>\n");
         }
         else
         {
            xml.append(" <message name=\"" + port + "_" + name + "\">\n");

            for(int t = 0; t < types.length; t++)
            {
               Class type = types[t];
               String paramName = type.getSimpleName() + "_" + (t + 1);
               
               String xsdtype = (String)mClassToXsdType.get(type);
               
               xml.append("  <part name=\"" + paramName + "\" " +
                          "type=\"" + xsdtype + "\"/>\n");
            }
            
            // end the message
            xml.append(" </message>\n");
         }
         
         Class resultClass = m.getReturnType();
         String resultType = (String)mClassToXsdType.get(resultClass);
         
         // add the result message
         xml.append(" <message name=\"" + port + "_" + name + "Response\">\n");
         xml.append("  <part name=\"result\" type=\"" + resultType + "\"/>\n");
         xml.append(" </message>\n");
      }
      
      return xml.toString();
   }
   
   /**
    * Generates xml for port type.
    * 
    * @param port the port type.
    * @param wsInterface the web service interface.
    * 
    * @return the xml for port type.
    */
   protected String generatePortTypeXml(String port, Class wsInterface)   
   {
      StringBuffer xml = new StringBuffer();
      
      xml.append(" <portType name=\"" + port + "\">\n");
      
      Method[] methods = wsInterface.getDeclaredMethods();
      for(int i = 0; i < methods.length; i++)
      {
         Method m = methods[i];
         String name = m.getName();
         
         xml.append("  <operation name=\"" + name + "\"");

         // build param order
         boolean putComma = false;
         String paramOrder = "";
         Class[] types = m.getParameterTypes();
         for(int t = 0; t < types.length; t++)
         {
            if(!putComma)
            {
               putComma = true;
            }
            else
            {
               paramOrder += ",";
            }

            paramOrder += types[t].getSimpleName() + "_" + (t + 1);
         }
         
         if(!paramOrder.equals(""))
         {
            xml.append(" parameterOrder=\"" + paramOrder + "\">\n");
         }
         else
         {
            xml.append(">\n");
         }
         
         xml.append("   <input message=\"tns:" + port + "_" + name + "\"/>\n");
         xml.append("   <output message=\"tns:" + port + "_" + name +
                    "Response\"/>\n");
         xml.append("  </operation>\n");
      }
      
      xml.append(" </portType>\n");
      
      return xml.toString();
   }
   
   /**
    * Generates xml for binding.
    * 
    * @param port the port type.
    * @param namespace the namespace.
    * @param wsInterface the web service interface.
    * 
    * @return the xml for binding.
    */
   protected String generateBindingXml(String port, String namespace,
                                       Class wsInterface)   
   {
      StringBuffer xml = new StringBuffer();
      
      xml.append(" <binding name=\"" + port + "SoapBinding\" " +
                 "type=\"tns:" + port + "\">\n");
      xml.append(" <soap:binding transport=\"" + HTTP_SCHEMA + "\" " +
                 "style=\"rpc\"/>\n");
      
      Method[] methods = wsInterface.getDeclaredMethods();
      for(int i = 0; i < methods.length; i++)
      {
         Method m = methods[i];
         String name = m.getName();
         
         xml.append("  <operation name=\"" + name + "\">\n");
         xml.append("   <soap:operation soapAction=\"\"/>\n");
         
         xml.append("   <input>\n");
         xml.append("    <soap:body " +
                    "encodingStyle=\"" + SoapMessage.ENCODING_SCHEMA + "\" " +
                    "use=\"encoded\" namespace=\"" + namespace + "\"/>\n");
         xml.append("   </input>\n");
         
         xml.append("   <output>\n");
         xml.append("    <soap:body " +
                    "encodingStyle=\"" + SoapMessage.ENCODING_SCHEMA + "\" " +
                    "use=\"encoded\" namespace=\"" + namespace + "\"/>\n");
         xml.append("   </output>\n");
         
         xml.append("  </operation>\n");
      }

      xml.append(" </binding>\n");

      return xml.toString();
   }
   
   /**
    * Generates a WSDL.
    * 
    * @param name the name of the soap server.
    * @param namespace the namespace for the soap handler.
    * @param portType the port type.
    * @param uri the uri for the soap server.
    * @param wsInterface the web service interface.
    * 
    * @return the generated WSDL.
    */
   public String generateWsdl(
      String name, String namespace, String portType, String uri,
      Class wsInterface)
   {
      StringBuffer wsdl = new StringBuffer();
      
      wsdl.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
      
      // definitions opening tag
      wsdl.append(
         "<definitions " +
         "xmlns=\"" + WSDL_NAMESPACE + "\" " +
         "xmlns:tns=\"" + namespace + "\" " +
         "xmlns:xsd=\"" + SoapMessage.XSD_NAMESPACE + "\" " +
         "xmlns:soap=\"" + SoapMessage.SOAP_NAMESPACE + "\" " +
         "name=\"" + name + "\" targetNamespace=\"" + namespace + "\">\n");
      
      // types
      wsdl.append(" <types/>\n");
      
      // messages
      wsdl.append(generateMessagesXml(portType, wsInterface));
      
      // port type information
      wsdl.append(generatePortTypeXml(portType, wsInterface));

      // generate binding
      wsdl.append(generateBindingXml(portType, namespace, wsInterface));
      
      wsdl.append(" <service " +
                  "name=\"" + name + "\">\n");
      wsdl.append("  <port " +
                  "name=\"" + portType + "Port\" " +
                  "binding=\"tns:" + portType + "SoapBinding\">\n");
      wsdl.append("   <soap:address location=\"" + uri + "\" " +
                  "xmlns=\"" + WSDL_NAMESPACE + "\"/>\n");
      wsdl.append("  </port>\n");
      wsdl.append(" </service>\n");
      
      wsdl.append("</definitions>");
      
      return wsdl.toString();
   }
   
   /**
    * Generates a WSDL.
    *
    * @param gsh the generic soap handler to generate the WSDL for. 
    * 
    * @return the generated WSDL.
    */
   public String generateWsdl(GenericSoapWebService gsh)   
   {
      String wsdl = generateWsdl(
         gsh.getName(), gsh.getNamespace(), gsh.getPortType(), gsh.getURI(),
         gsh.getSoapInterface());
      
      return wsdl;
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
}
