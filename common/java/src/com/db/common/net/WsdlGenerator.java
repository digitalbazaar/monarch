/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.net;

import com.db.common.logging.Logger;
import com.db.common.logging.LoggerManager;

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
    * Constants.
    */
   public static final String WSDL_NAMESPACE =
      "http://schemas.xmlsoap.org/wsdl/";
   
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
    * @return the xml for messages.
    */
   protected String generateMessagesXml(String port, Class wsInterface)
   {
      String xml = "";
      
      Method[] methods = wsInterface.getDeclaredMethods();
      for(int i = 0; i < methods.length; i++)
      {
         Method m = methods[i];
         String name = m.getName();
         
         Class[] types = m.getParameterTypes();
         if(types.length == 0)
         {
            xml += " <message name=\"" + port + "_" + name + "\"/>\n";
         }
         else
         {
            xml += " <message name=\"" + port + "_" + name + "\">\n";

            for(int t = 0; t < types.length; t++)
            {
               Class type = types[t];
               String paramName = type.getSimpleName() + "_" + (t + 1);
               
               String xsdtype = (String)mClassToXsdType.get(type);
               
               xml +=
                  "  <part name=\"" + paramName + "\" " +
                  "type=\"" + xsdtype + "\"/>\n";
            }
            
            // end the message
            xml += " </message>\n";
         }
         
         Class resultClass = m.getReturnType();
         String resultType = (String)mClassToXsdType.get(resultClass);
         
         // add the result message
         xml += " <message name=\"" + port + "_" + name + "Response\">\n";
         xml += "  <part name=\"result\" type=\"" + resultType + "\"/>\n";
         xml += " </message>\n";
      }
      
      return xml;
   }
   
   /**
    * Generates xml for port type.
    * 
    * @param port the port type.
    * @param wsInterface the web service interface.
    * @return the xml for port type.
    */
   protected String generatePortTypeXml(String port, Class wsInterface)   
   {
      String xml = "";
      
      xml += " <portType name=\"" + port + "\">\n";
      
      Method[] methods = wsInterface.getDeclaredMethods();
      for(int i = 0; i < methods.length; i++)
      {
         Method m = methods[i];
         String name = m.getName();
         
         xml += "  <operation name=\"" + name + "\"";

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
            xml += " parameterOrder=\"" + paramOrder + "\">\n";
         }
         else
         {
            xml += ">\n";
         }
         
         xml += "   <input message=\"tns:" + port + "_" + name + "\"/>\n";
         xml += "   <output message=\"tns:" + port + "_" + name +
                "Response\"/>\n";
         xml += "  </operation>\n";
      }
      
      xml += " </portType>\n";
      
      return xml;
   }
   
   /**
    * Generates xml for binding.
    * 
    * @param port the port type.
    * @param namespace the namespace.
    * @param wsInterface the web service interface.
    * @return the xml for binding.
    */
   protected String generateBindingXml(String port, String namespace,
                                       Class wsInterface)   
   {
      String xml = "";
      
      xml += " <binding name=\"" + port + "Binding\" " +
             "type=\"tns:" + port + "\">\n";
      xml += " <soap:binding transport=\"" + HTTP_SCHEMA + "\" " +
             "style=\"rpc\"/>\n";
      
      Method[] methods = wsInterface.getDeclaredMethods();
      for(int i = 0; i < methods.length; i++)
      {
         Method m = methods[i];
         String name = m.getName();
         
         xml += "  <operation name=\"" + name + "\">\n";
         xml += "   <soap:operation soapAction=\"\"/>\n";
         
         xml += "   <input>\n";
         xml += "    <soap:body " +
                "encodingStyle=\"" + SoapMessage.ENCODING_SCHEMA + "\" " +
                "use=\"encoded\" namespace=\"" + namespace + "\"/>\n";
         xml += "   </input>\n";
         
         xml += "   <output>\n";
         xml += "    <soap:body " +
                "encodingStyle=\"" + SoapMessage.ENCODING_SCHEMA + "\" " +
                "use=\"encoded\" namespace=\"" + namespace + "\"/>\n";
         xml += "   </output>\n";
         
         xml += "  </operation>\n";
      }

      xml += " </binding>\n";

      return xml;
   }
   
   /**
    * Generates a WSDL.
    * 
    * @param name the name of the soap server.
    * @param namespace the namespace for the soap handler.
    * @param portType the port type.
    * @param uri the uri for the soap server.
    * @param wsInterface the web service interface.
    * @return the generated WSDL.
    */
   public String generateWsdl(String name, String namespace, String portType,
                              String uri, Class wsInterface)
   {
      String wsdl = "";
      
      wsdl += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      
      // definitions opening tag
      wsdl +=
         "<definitions " +
         "xmlns=\"" + WSDL_NAMESPACE + "\" " +
         "xmlns:tns=\"" + namespace + "\" " +
         "xmlns:xsd=\"" + SoapMessage.XSD_NAMESPACE + "\" " +
         "xmlns:soap=\"" + SoapMessage.SOAP_NAMESPACE + "\" " +
         "name=\"" + name + "\" targetNamespace=\"" + namespace + "\">\n";
      
      // types
      wsdl += " <types/>\n";
      
      // messages
      wsdl += generateMessagesXml(portType, wsInterface);
      
      // port type information
      wsdl += generatePortTypeXml(portType, wsInterface);

      // generate binding
      wsdl += generateBindingXml(portType, namespace, wsInterface);
      
      wsdl += " <service " +
              "name=\"" + name + "\">\n";
      wsdl += "  <port " +
              "name=\"" + portType + "Port\" " +
              "binding=\"tns:" + portType + "Binding\">\n";
      wsdl += "   <soap:address location=\"" + uri + "\" " +
              "xmlns=\"" + WSDL_NAMESPACE + "\"/>\n";
      wsdl += "  </port>\n";
      wsdl += " </service>\n";
      
      wsdl += "</definitions>";      
      
      return wsdl;
   }
   
   /**
    * Generates a WSDL.
    *
    * @param gsh the generic soap handler to generate the WSDL for. 
    * @return the generated WSDL.
    */
   public String generateWsdl(GenericSoapWebService gsh)   
   {
      String wsdl = generateWsdl(gsh.getName(), gsh.getNamespace(),
                                 gsh.getPortType(), gsh.getURI(),
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
      return LoggerManager.getLogger("dbcommon");
   }
}
