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
 * An AbstractSoapWebService provides all of the basic underlying soap
 * functionality needed to provide a secure soap web service. All that
 * is needed by extending classes is an interface that defines the
 * soap methods (and therefore port type) for the web service along
 * with a name and namespace for the web service and its definitions.
 * 
 * @author Dave Longley
 */
public abstract class AbstractSoapWebService implements SecureSoapWebService
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
    * The SOAP binding for this web service.
    */
   protected WsdlSoapBinding mSoapBinding;
   
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
    * Creates an AbstractSoapWebService. The soap implementer must be this
    * object.
    * 
    * @param name the name of this web service.
    * @param namespace the namespace for this web service.
    * @param soapInterface the soap interface for the web service.
    */
   public AbstractSoapWebService(
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
      
      // get the soap binding for this service
      mSoapBinding = (WsdlSoapBinding)mWsdl.getBindings().
         getBinding(mPortType + "SoapBinding");
   }

   /**
    * Creates an AbstractSoapWebService. Use this constructor if you wish
    * the soap implementer to be a different object than this one.
    * 
    * @param name the name of this web service.
    * @param namespace the namespace for this web service.
    * @param soapImplementer the implementing object of the soap interface.
    * @param soapInterface the soap interface for the web service.
    */
   public AbstractSoapWebService(
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
      
      // get the soap binding for this service
      mSoapBinding = (WsdlSoapBinding)mWsdl.getBindings().
         getBinding(mPortType + "SoapBinding");
   }
   
   /**
    * Gets the parameters for a SoapOperation as an array of objects.
    * 
    * @param operation the SoapOperation.
    * 
    * @return the parameters as an array of objects.
    * 
    * @exception SoapMethodNotRecognizedException thrown if the operation is
    *                                             not recognized.
    */
   protected Object[] getParameterArray(SoapOperation operation)
   throws SoapMethodNotRecognizedException
   {
      Object[] params = new Object[0];
      
      try
      {
         // get the soap binding operation
         WsdlSoapBindingOperation bindingOperation =
            getSoapBinding().getOperations().getOperation(operation.getName());
         
         // get the parameters
         params = bindingOperation.getParameterArray(operation);
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "could not invoke soap method: " + operation.getName() +
            ",soap method not recognized!");
         throw new SoapMethodNotRecognizedException(
            "Could not invoke soap method: " + operation.getName() +
            ",soap method not recognized!", t);
      }
      
      return params;
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
    * @exception Throwable thrown if some exception occurs.
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
      
      String ip = sm.getRemoteIP();
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
   public void checkSoapSecurity(RpcSoapMessage sm) throws SecurityException
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
   public Object callSoapMethod(RpcSoapMessage sm)
   {
      Object rval = null;
      
      long st = System.currentTimeMillis();

      Thread thread = Thread.currentThread();
      
      // get the soap envelope from the message
      RpcSoapEnvelope envelope = sm.getRpcSoapEnvelope();

      // see the envelope has an operation to perform
      if(envelope.containsSoapOperation())
      {
         // get the soap operation
         SoapOperation operation = envelope.getSoapOperation();
         
         getLogger().debug(getClass(),
            "attempting to call soap method: " + operation.getName());
      
         try
         {
            // add the current thread to the thread->soap message map
            mCallThreadToSoapMessage.put(thread, sm);
            
            // check soap security before invoking method
            checkSoapSecurity(sm);
            
            // get the method parameters
            Object[] params = getParameterArray(operation);
            
            // invoke the soap method
            rval = invokeSoapMethod(operation.getName(), params);
            
            // remove the thread from the thread->soap message map
            mCallThreadToSoapMessage.remove(thread);

            // set soap result
            Object[] results = null;
            if(rval != null)
            {
               results = new Object[]{rval};
            }
            
            // get the soap binding operation
            WsdlSoapBindingOperation bindingOperation =
               getSoapBinding().getOperations().getOperation(
                  operation.getName());
            
            // create a response soap operation
            SoapOperation responseOperation =
               bindingOperation.createResponseSoapOperation(results);
            
            // set the envelope's soap operation
            envelope.setSoapOperation(responseOperation);
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
               SoapFault fault = new SoapFault();
               fault.setFaultCode(SoapFault.FAULT_CLIENT);
               fault.setFaultString(
                  "The soap method was not recognized by the server. Check " +
                  "the method signature and parameter names.");
               fault.setFaultActor(getURI());
               
               envelope.setSoapFault(fault);
            }
            else if(t instanceof SecurityException)
            {
               // if the soap message doesn't already contain a fault, create
               // a default one
               if(!envelope.containsSoapFault())
               {
                  SoapFault fault = new SoapFault();
                  fault.setFaultCode(SoapFault.FAULT_CLIENT);
                  fault.setFaultString(
                     "The client was not authorized to perform the " +
                     "requested action.");
                  fault.setFaultActor(getURI());
                  
                  envelope.setSoapFault(fault);
               }
            }
            else
            {
               SoapFault fault = new SoapFault();
               fault.setFaultCode(SoapFault.FAULT_SERVER);
               fault.setFaultString(
                  "An exception was thrown by the server " +
                  "when calling the specified soap method.");
               fault.setFaultActor(getURI());
               
               envelope.setSoapFault(fault);
            }
            
            SoapFault fault = envelope.getSoapFault();
            getLogger().debug(getClass(), 
               "failed to call soap method, sending soap fault" +
               ",reason=" + fault.getFaultString());
            
            getLogger().debug(getClass(), Logger.getStackTrace(t));
         }
         
         long et = System.currentTimeMillis();
         getLogger().debug(getClass(),
            "total soap method (" + operation.getName() + ") time: " +
            (et - st) + " ms");
      }
      else
      {
         // soap operation was not found in the soap message
         SoapFault fault = new SoapFault();
         fault.setFaultCode(SoapFault.FAULT_CLIENT);
         fault.setFaultString(
            "No soap operation was found in the soap message.");
         fault.setFaultActor(getURI());
         
         envelope.setSoapFault(fault);         
      }
      
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
      // abstract soap web service always returns true
      return true;
   }
   
   /**
    * Creates a soap message for use with this service.
    * 
    * @return a soap message for use with this service.
    */
   public RpcSoapMessage createSoapMessage()   
   {
      RpcSoapMessage sm = new RpcSoapMessage();
      SoapPermission permission = new SoapPermission("envelope.log");
      sm.setSoapEnvelopeLoggingPermitted(checkSoapPermission(permission));
      return sm;
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
    * Gets the SOAP binding for this web service.
    * 
    * @return the SOAP binding for this web service.
    */
   public WsdlSoapBinding getSoapBinding()
   {
      return mSoapBinding;
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
    * Sets the namespace URI for the web service.
    * 
    * @param namespaceUri URI the web service.
    */
   public void setNamespaceUri(String namespaceUri)
   {
      getWsdl().setTargetNamespaceUri(namespaceUri);
   }
   
   /**
    * Gets the namespace URI for the web service.
    * 
    * @return the namespace URI for the web service.
    */
   public String getNamespaceUri()
   {
      return getWsdl().getTargetNamespaceUri();
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
    * Returns true if the passed permission is allowed, false if not.
    *
    * @param permission a SoapPermission to check.
    *
    * @return true if the passed permission is allowed, false if not.
    */
   public boolean checkSoapPermission(SoapPermission permission)   
   {
      boolean rval = true;
      
      if(getSoapSecurityManager() != null)
      {
         rval = getSoapSecurityManager().checkSoapPermission(permission);
      }
      
      return rval;
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
