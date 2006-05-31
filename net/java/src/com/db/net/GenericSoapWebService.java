/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A generic soap web service.
 * 
 * @author Dave Longley
 */
public abstract class GenericSoapWebService implements SoapWebService
{
   /**
    * The soap interface. 
    */
   protected Class mSoapInterface;
   
   /**
    * The implementer of the soap interface.
    */
   protected Object mSoapImplementer;
   
   /**
    * The name of the web service.
    */
   protected String mName;
   
   /**
    * The name space for the web service.
    */
   protected String mNamespace;
   
   /**
    * The port type for the web service.
    */
   protected String mPortType;
   
   /**
    * The uri for the soap web service.
    */
   protected String mURI;
   
   /**
    * The path to the wsdl.
    */
   protected String mWsdlPath;
   
   /**
    * A map of threads executing soap methods to the soap messages
    * that they are processing.
    */
   protected Hashtable mCallThreadToSoapMessage;
   
   /**
    * Creates a generic soap web service. The soap implementer must be this
    * object.
    * 
    * @param soapInterface the soap interface for the web service.
    */
   public GenericSoapWebService(Class soapInterface) 
   {
      mSoapImplementer = this;
      mSoapInterface = soapInterface;
      mName = "";
      mNamespace = "";
      mPortType = "";
      mURI = "";
      mWsdlPath = "";
      mCallThreadToSoapMessage = new Hashtable();
   }

   /**
    * Creates a generic soap web service. Use this constructor if you wish
    * the soap implementer to be a different object than this one.
    * 
    * @param soapImplementer the implementing object of the soap interface.
    * @param soapInterface the soap interface for the web service.
    */
   public GenericSoapWebService(Object soapImplementer, Class soapInterface) 
   {
      mSoapImplementer = soapImplementer;
      mSoapInterface = soapInterface;
      mName = "";
      mNamespace = "";
      mPortType = "";
      mURI = "";
      mWsdlPath = "";
   }

   /**
    * Invokes a method from the soap interface on the soap implementer. Throws
    * an exception if the soap method could not be invoked.
    * 
    * @param method the method to call.
    * @param params the params for the method.
    * @return the return value from the method.
    * @throws Throwable 
    */
   protected Object invokeSoapMethod(String method, Vector params)
   throws Throwable
   {
      Object rval = null;
      
      boolean methodInvoked = false;
      Method[] methods = getSoapInterface().getDeclaredMethods();
      for(int i = 0; i < methods.length; i++)
      {
         Method m = methods[i];
         if(m.getName().equals(method))
         {
            getLogger().debug("soap method name match,method=" + method);

            Class[] types = m.getParameterTypes();
            int numParams = types.length;
            if(params.size() == numParams)
            {
               getLogger().debug("parameter count match,count=" +
                                 params.size());

               try
               {
                  rval = m.invoke(getSoapImplementer(), params.toArray());
                  methodInvoked = true;
               }
               catch(Throwable t)
               {
                  getLogger().error("could not invoke soap method: " + method +
                                    ",an exception occured.");
                  throw t;
               }
               
               break;
            }

            getLogger().debug("parameter count discrepancy" +
                  ",soap method parameter count=" + numParams +
                  ",passed parameter count=" + params.size());
         }
      }

      if(!methodInvoked)
      {
         getLogger().error("could not invoke soap method: " + method +
                           ",soap method not recognized!");
         throw new SoapMethodNotRecognizedException(
               "Could not invoke soap method: " + method +
               ",soap method not recognized!");
      }
      
      return rval;
   }
   
   /**
    * Converts the parameter for a soap method to the appropriate
    * type of object.
    * 
    * @param param the param string for a soap method.
    * @param paramType the appropriate type of the result.
    * @return the param converted to the appropriate type.
    */
   protected Object convertParam(String param, Class paramType)
   {
      Object rval = null;
      
      getLogger().debug("param type: " + paramType);
      
      try
      {
         rval = WsdlParser.parseObject(param, paramType);
      }
      catch(Throwable t)
      {
         getLogger().error("param invalid");
      }
      
      return rval;
   }
   
   /**
    * Gets the soap message, if any, for the client whose soap request
    * is being processed by this thread.
    * 
    * @return the soap message of the client of null if no client is
    *         connected.
    */
   public SoapMessage getSoapMessage()
   {
      SoapMessage rval = null;
      
      // get the soap message
      rval = (SoapMessage)mCallThreadToSoapMessage.get(Thread.currentThread());
      
      return rval;
   }
   
   /**
    * Gets the client IP, if any, for the client whose soap request
    * is being processed by this thread.
    * 
    * @return the ip address of the client or 0.0.0.0 if no client connected.
    */
   public String getClientIP()
   {
      String clientIP = "0.0.0.0";
      
      // get the soap message
      SoapMessage sm =
         (SoapMessage)mCallThreadToSoapMessage.get(Thread.currentThread());
      
      String ip = sm.getClientIP();
      if(ip != null)
      {
         clientIP = ip;
      }
      
      return clientIP;
   }
   
   /**
    * Gets a vector of ordered parameters for a soap method from a
    * mapping of parameter name to value.
    * 
    * @param method the method to get the ordered params for.
    * @param params a mapping of parameter name to value.
    * @return a vector of ordered parameter values.
    */
   public Vector getOrderedParams(String method, HashMap params)
   {
      Vector orderedParams = new Vector();

      // build paramOrder
      StringBuffer paramOrder = new StringBuffer();
      Method[] methods = getSoapInterface().getDeclaredMethods();
      Class[] types = new Class[0];
      for(int i = 0; i < methods.length; i++)
      {
         Method m = methods[i];
         if(m.getName().equals(method))
         {
            // get the param types
            types = m.getParameterTypes();
            int numParams = types.length;
            if(params.size() == numParams)
            {
               for(int t = 0; t < types.length; t++)
               {
                  if(paramOrder.length() != 0)
                  {
                     paramOrder.append(',');
                  }

                  paramOrder.append(types[t].getSimpleName());
                  paramOrder.append('_');
                  paramOrder.append((t + 1));
               }
            }
            
            break;
         }
      }
      
      String[] paramNames = paramOrder.toString().split(",");
      for(int i = 0; i < paramNames.length && i < types.length; i++)
      {
         String value = (String)params.get(paramNames[i]);
         if(value != null)
         {
            // convert the value string into the appropriate type
            Object param = convertParam(value, types[i]); 
            orderedParams.add(param);
         }
      }
      
      return orderedParams;
   }

   /**
    * Calls the appropriate soap method.
    * 
    * @param sm the soap message.
    * @return the return value from the called method.
    */
   public Object callSoapMethod(SoapMessage sm)
   {
      Object rval = null;
      
      long st = new java.util.Date().getTime();

      Thread thread = Thread.currentThread();
      
      String method = sm.getMethod();
      getLogger().debug("attempting to call soap method: " + method);
      
      try
      {
         // add the current thread to the thread->soap message map
         mCallThreadToSoapMessage.put(thread, sm);
         
         // get the parameters in order
         Vector params = getOrderedParams(method, sm.getParams());
            
         // invoke the soap method
         rval = invokeSoapMethod(method, params);
         
         // remove the thread from the thread->soap message map
         mCallThreadToSoapMessage.remove(thread);

         // set soap message result
         sm.setResult("" + rval);
         sm.setSerializerOptions(SoapMessage.SOAP_RESPONSE);
      }
      catch(Throwable t)
      {
         // remove the thread from the thread->soap message map
         if(thread != null)
         {
            mCallThreadToSoapMessage.remove(thread);
         }

         getLogger().debug("failed to call soap method, sending soap fault.");

         // set soap fault
         if(t instanceof SoapMethodNotRecognizedException)
         {
            sm.setFaultCode(SoapMessage.FAULT_CLIENT);
            sm.setFaultString("The soap method was not recognized by the " +
                              "server. Check the method signature and " +
                              "parameter names.");
            sm.setFaultActor(getURI());
         }
         else
         {
            sm.setFaultCode(SoapMessage.FAULT_SERVER);
            sm.setFaultString("An exception was thrown by the server " +
                              "when calling the specified soap method.");
            sm.setFaultActor(getURI());
         }

         getLogger().debug(Logger.getStackTrace(t));
      }
      
      long et = new java.util.Date().getTime();
      getLogger().debug("total soap method (" + method + ") time: " +
                        (et - st) + " ms");
      
      return rval;
   }
   
   /**
    * Returns true if the passed soap action is valid for this service.
    *
    * @param action the soap action to check.
    * @return true if the passed soap action is valid, false if not.
    */
   public boolean isSoapActionValid(String action)
   {
      // generic soap web service always returns true
      return true;
   }   

   /**
    * Gets the WSDL as a string.
    * 
    * @return the WSDL as a string.
    */
   public String getWsdl()
   {
      String wsdl = "";
      
      WsdlGenerator wsdlGen = new WsdlGenerator();
      wsdl = wsdlGen.generateWsdl(this);
      
      return wsdl;
   }
   
   /**
    * Gets the soap interface implementer.
    * 
    * @return the soap interface implenter.
    */
   public Object getSoapImplementer()
   {
      return mSoapImplementer;
   }
   
   /**
    * Gets the soap interface.
    * 
    * @return the soap interface.
    */
   public Class getSoapInterface()
   {
      return mSoapInterface;
   }
   
   /**
    * Sets the name for the web service.
    * 
    * @param name the name.
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
    * @param namespace the web service.
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
    * Sets the uri for the soap web service.
    * 
    * @param uri the uri for the soap web service.
    */
   public void setURI(String uri)
   {
      mURI = uri;
   }
   
   /**
    * Gets the uri for the soap server.
    * 
    * @return the uri for the soap server.
    */
   public String getURI()
   {
      return mURI;
   }
   
   /**
    * Sets the WSDL path for this handler.
    * 
    * @param wsdlPath the wsdl path for this handler.
    */
   public void setWsdlPath(String wsdlPath)
   {
      mWsdlPath = wsdlPath;
   }
   
   /**
    * Gets the path to the WSDL.
    * 
    * @return the WSDL path.
    */
   public String getWsdlPath()
   {
      return mWsdlPath;
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
    * An exception to be thrown when a soap method is not recognized.
    */
   public class SoapMethodNotRecognizedException extends Exception
   {
      /**
       * Creates a new SoapMethodNotRecognized exception.
       * 
       * @param message a detailed message describing the exception.
       */
      public SoapMethodNotRecognizedException(String message)
      {
         super(message);
      }

      /**
       * Creates a new SoapMethodNotRecognized exception.
       * 
       * @param message a detailed message describing the exception.
       * @param t the throwable cause.
       */
      public SoapMethodNotRecognizedException(String message, Throwable t)
      {
         super(message, t);
      }
   }
}
