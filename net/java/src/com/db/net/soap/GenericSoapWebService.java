/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import java.lang.reflect.Method;
import java.util.HashMap;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.wsdl.Wsdl;
import com.db.net.wsdl.WsdlService;
import com.db.util.MethodInvoker;

/**
 * A generic soap web service.
 * 
 * @author Dave Longley
 */
public abstract class GenericSoapWebService implements SecureSoapWebService
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
    * The port type for the web service.
    */
   protected String mPortType;
   
   /**
    * The path to the wsdl.
    */
   protected String mWsdlPath;
   
   /**
    * The Wsdl for this web service.
    */
   protected Wsdl mWsdl;
   
   /**
    * A map of threads executing soap methods to the soap messages
    * that they are processing.
    */
   protected HashMap mCallThreadToSoapMessage;
   
   /**
    * The soap security manager for this soap web service. 
    */
   protected SoapSecurityManager mSoapSecurityManager;
   
   /**
    * Creates a generic soap web service. The soap implementer must be this
    * object.
    * 
    * @param name the name of this web service.
    * @param namespace the namespace for this web service.
    * @param soapInterface the soap interface for the web service.
    */
   public GenericSoapWebService(
      String name, String namespace, Class soapInterface) 
   {
      mSoapImplementer = this;
      mSoapInterface = soapInterface;
      mPortType = soapInterface.getSimpleName();
      mWsdlPath = "";
      mWsdl = new Wsdl(
         name, namespace, soapInterface,
         new WsdlSoapBindingFactory(), new WsdlSoapPortFactory());
      mCallThreadToSoapMessage = new HashMap();
      
      // install no soap security manager
      setSoapSecurityManager(null);
   }

   /**
    * Creates a generic soap web service. Use this constructor if you wish
    * the soap implementer to be a different object than this one.
    * 
    * @param name the name of this web service.
    * @param namespace the namespace for this web service.
    * @param soapImplementer the implementing object of the soap interface.
    * @param soapInterface the soap interface for the web service.
    */
   public GenericSoapWebService(
      String name, String namespace,
      Object soapImplementer, Class soapInterface) 
   {
      mSoapImplementer = soapImplementer;
      mSoapInterface = soapInterface;
      mPortType = soapInterface.getSimpleName();
      mWsdlPath = "";
      mWsdl = new Wsdl(
         name, namespace, soapInterface,
         new WsdlSoapBindingFactory(), new WsdlSoapPortFactory());
      mCallThreadToSoapMessage = new HashMap();
      
      // install no soap security manager
      setSoapSecurityManager(null);
   }

   /**
    * Invokes a method from the soap interface on the soap implementer. Throws
    * an exception if the soap method could not be invoked.
    * 
    * @param methodName the name of the method to call.
    * @param params the parameters for the method.
    * 
    * @return the return value from the method.
    * 
    * @throws Throwable 
    */
   protected Object invokeSoapMethod(String methodName, Object[] params)
   throws Throwable
   {
      Object rval = null;
      
      // find the soap method
      Method method = MethodInvoker.findMethod(
         getSoapInterface(), methodName, params);
      
      if(method != null)
      {
         // get method signature
         String signature = MethodInvoker.getSignature(method);
         
         getLogger().detail(getClass(),
            "soap method found,method=" + signature);
         
         try
         {
            // invoke method
            rval = method.invoke(getSoapImplementer(), params);
         }
         catch(Throwable t)
         {
            getLogger().error(getClass(),
               "could not invoke soap method '" + signature +
               "',an exception occured,exception= " + t);
            throw t;
         }
      }
      else
      {
         getLogger().error(getClass(),
            "could not invoke soap method: " + methodName +
            ",soap method not recognized!");
         throw new SoapMethodNotRecognizedException(
            "Could not invoke soap method: " + methodName +
            ",soap method not recognized!");
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
    * Checks to see if the passed soap message passes security. Extending
    * classes should implement this method to set their particular security
    * profile.
    * 
    * When this method is called, a client is connected and awaiting a soap
    * response. Information about the client (i.e. its IP address) can be
    * checked in this method and a security exception can be thrown if
    * appropriate.
    * 
    * If there is a particular security policy for certain soap methods this
    * method should handle that policy and throw a security exception if
    * appropriate.
    * 
    * A soap fault will be raised that indicates that the client was not
    * authenticated if a security exception is thrown from this method.
    * 
    * @param sm the soap message sent by the client.
    * 
    * @exception SecurityException thrown if the client or the soap message
    *                              sent by the client do not pass security.
    */
   public void checkSoapSecurity(SoapMessage sm) throws SecurityException
   {
      // get the security manager, if one exists
      if(getSoapSecurityManager() != null)
      {
         // check security
         getSoapSecurityManager().checkSoapSecurity(sm);
      }
   }

   /**
    * Calls the appropriate soap method.
    * 
    * @param sm the soap message.
    * 
    * @return the return value from the called method.
    */
   public Object callSoapMethod(SoapMessage sm)
   {
      Object rval = null;
      
      long st = System.currentTimeMillis();

      Thread thread = Thread.currentThread();
      
      String method = sm.getMethod();
      getLogger().debug(getClass(),
         "attempting to call soap method: " + method);
      
      try
      {
         // add the current thread to the thread->soap message map
         mCallThreadToSoapMessage.put(thread, sm);
         
         // check soap security before invoking method
         checkSoapSecurity(sm);
         
         // invoke the soap method
         rval = invokeSoapMethod(method, sm.getParameters());
         
         // remove the thread from the thread->soap message map
         mCallThreadToSoapMessage.remove(thread);

         // set soap message result
         Object[] results = null;
         if(rval != null)
         {
            results = new Object[]{rval};
         }
         
         sm.setResults(results);
         sm.setXmlSerializerOptions(SoapMessage.SOAP_RESPONSE);
      }
      catch(Throwable t)
      {
         // remove the thread from the thread->soap message map
         if(thread != null)
         {
            mCallThreadToSoapMessage.remove(thread);
         }

         // set soap fault
         if(t instanceof SoapMethodNotRecognizedException)
         {
            sm.setFaultCode(SoapMessage.FAULT_CLIENT);
            sm.setFaultString("The soap method was not recognized by the " +
                              "server. Check the method signature and " +
                              "parameter names.");
            sm.setFaultActor(getURI());
         }
         else if(t instanceof SecurityException)
         {
            // if the soap message isn't already a fault, create a default one
            if(!sm.isFault())
            {
               sm.setFaultCode(SoapMessage.FAULT_CLIENT);
               sm.setFaultString(
                  "The client was not authorized to perform the " +
                  "requested action.");
               sm.setFaultActor(getURI());
            }
         }
         else
         {
            sm.setFaultCode(SoapMessage.FAULT_SERVER);
            sm.setFaultString("An exception was thrown by the server " +
                              "when calling the specified soap method.");
            sm.setFaultActor(getURI());
         }
         
         getLogger().debug(getClass(), 
            "failed to call soap method, sending soap fault" +
            ",reason=" + sm.getFaultString());

         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      long et = System.currentTimeMillis();
      getLogger().debug(getClass(),
         "total soap method (" + method + ") time: " + (et - st) + " ms");
      
      return rval;
   }
   
   /**
    * Returns true if the passed soap action is valid for this service.
    *
    * @param action the soap action to check.
    * 
    * @return true if the passed soap action is valid, false if not.
    */
   public boolean isSoapActionValid(String action)
   {
      // generic soap web service always returns true
      return true;
   }
   
   /**
    * Creates a soap message for use with this service.
    * 
    * @return a soap message for use with this service.
    */
   public SoapMessage createSoapMessage()   
   {
      return new SoapMessage(getWsdl(), getPortType());
   }

   /**
    * Gets the WSDL.
    * 
    * @return the WSDL.
    */
   public Wsdl getWsdl()
   {
      return mWsdl;
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
      getWsdl().setName(name);
   }
   
   /**
    * Gets the name for the web service.
    * 
    * @return the name for the web service.
    */
   public String getName()
   {
      return getWsdl().getName();
   }
   
   /**
    * Sets the namespace for the web service.
    * 
    * @param namespace the web service.
    */
   public void setNamespace(String namespace)
   {
      getWsdl().setTargetNamespace(namespace);
   }
   
   /**
    * Gets the namespace for the web service.
    * 
    * @return the namespace for the web service.
    */
   public String getNamespace()
   {
      return getWsdl().getTargetNamespace();
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
    * Gets the WSDL port type.
    * 
    * @return the WSDL port type.
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
      // get the service
      WsdlService service = getWsdl().getServices().getService(getName());
      
      // get the port
      WsdlSoapPort port =
         (WsdlSoapPort)service.getPorts().getPort(getPortType() + "Port");
      
      // set the uri
      port.setUri(uri);
   }
   
   /**
    * Gets the uri for the soap server.
    * 
    * @return the uri for the soap server.
    */
   public String getURI()
   {
      // get the service
      WsdlService service = getWsdl().getServices().getService(getName());
      
      // get the port
      WsdlSoapPort port =
         (WsdlSoapPort)service.getPorts().getPort(getPortType() + "Port");
      
      // return the uri
      return port.getUri();
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
    * Sets the soap security manager for this soap web service. The soap
    * security manager can only be set once.
    * 
    * @param ssm the soap sercurity manager for this soap web service.
    */
   public void setSoapSecurityManager(SoapSecurityManager ssm)
   {
      // only set soap security manager if it is null
      if(getSoapSecurityManager() == null)
      {
         mSoapSecurityManager = ssm;
      }
   }
   
   /**
    * Gets the soap security manager for this soap web service.
    * 
    * @return the soap security manager for this soap web service
    *         (can be null if there is no installed security manager).
    */
   public SoapSecurityManager getSoapSecurityManager()
   {
      return mSoapSecurityManager;
   }
   
   /**
    * Gets the logger for this soap web service.
    * 
    * @return the logger for this soap web service.
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
