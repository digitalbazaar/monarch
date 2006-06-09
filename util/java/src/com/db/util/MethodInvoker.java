/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

import java.lang.reflect.Method;

/**
 * This object is used to invoke a method, optionally, report a message back
 * when the method completes. The method can be invoked in another thread
 * or on the current one.
 * 
 * @author Dave Longley
 */
public class MethodInvoker extends Thread
{
   /**
    * The object with the method to invoke.
    */
   protected Object mAgent;
   
   /**
    * The name of the method to invoke.
    */
   protected String mMethodName;
   
   /**
    * The method to invoke.
    */
   protected Method mMethod;
   
   /**
    * The method parameters.
    */
   protected Object[] mParams;
   
   /**
    * A custom lock object to synchronize on.
    */
   protected Object mLockObject;
   
   /**
    * True if the lock object should remain locked until the method invoked
    * message has been handled, false if it should unlock immediately after
    * the method has been invoked.
    */
   protected boolean mLockMessageHandling;
   
   /**
    * A MethodInvokedMessage with information about the invoked method.
    */
   protected MethodInvokedMessage mMessage;
   
   /**
    * A MethodInvoker that calls a callback method once the method completes.
    */
   protected MethodInvoker mCallbackMethodInvoker;
   
   /**
    * The MethodInvokerListener to receive the MethodInvokedMessage once
    * the method completes.
    */
   protected MethodInvokerListener mListener;
   
   /**
    * Creates a new MethodInvoker.
    * 
    * @param agent the object that the method will be invoked on.
    * @param method the method to invoke.
    * @param params the parameters for the method.
    */
   public MethodInvoker(Object agent, Method method, Object[] params)
   {
      // throw an exception if the method is null
      if(method == null)
      {
         throw new IllegalArgumentException(
            "Method for MethodInvoker must not be null.");
      }
      
      // initialize
      initialize(agent, method.getName(), method, params);
   }

   /**
    * Creates a new MethodInvoker.
    * 
    * @param agent the object that the method will be invoked on.
    * @param methodName the name of the method to invoke.
    * @param params the parameters for the method.
    */
   public MethodInvoker(Object agent, String methodName, Object[] params)
   {
      // throw an exception if the method name is null or a blank string
      if(methodName == null || methodName.equals(""))
      {
         throw new IllegalArgumentException(
            "Method name for MethodInvoker must not be null " +
            "or a blank string.");
      }

      // initialize
      initialize(agent, methodName, null, params);
   }
   
   /**
    * Initializes this method invoker. 
    * 
    * @param agent the object that the method will be invoked on.
    * @param methodName the name of the method to invoke.
    * @param method the method to invoke (can be null).
    * @param params the parameters for the method.
    */
   protected void initialize(
      Object agent, String methodName, Method method, Object[] params)
   {
      // throw an exception if the agent is null
      if(agent == null)
      {
         throw new IllegalArgumentException(
            "Agent for MethodInvoker must not be null.");
      }
      
      mAgent = agent;
      mMethodName = methodName;
      mMethod = method;
      mParams = params;
      
      if(mParams == null)
      {
         mParams = new Object[0];
      }
      
      mLockObject = null;
      mLockMessageHandling = false;
      
      mMessage = null;
      mListener = null;
      mCallbackMethodInvoker = null;
      
      // set thread name
      String signature = getSignature(mMethodName, mParams);
      Class methodClass = getMethodClass(mAgent);
      setName("MethodInvoker for method '" + signature +
              "' in class '" + methodClass.getName() + "'");
   }
   
   /**
    * Invokes the method.
    * 
    * @param method the method to invoke.
    * @param agent the object to invoke the method on.
    * @param params the parameters for the method.
    * 
    * @return the return value from the method.
    */
   protected Object invokeMethod(Method method, Object agent, Object[] params) 
   {
      Object rval = null;
      
      // get signature string and agent name
      String signature = getSignature(method);
      Class methodClass = getMethodClass(agent);
      
      try
      {
         getLogger().debug(
            "invoking method: '" + signature +
            "' from class '" + methodClass.getName() + "'");
         
         // invoke method
         rval = method.invoke(agent, params);
      }
      catch(Throwable t)
      {
         if(mMessage != null)
         {
            mMessage.setMethodException(t);
         }
         
         getLogger().error(
            "an exception occurred while invoking method: '" + signature +
            "' from class: '" + methodClass.getName() + "'," +
            "\nexception= " + t +
            "\ncause= " + t.getCause() +
            "\ntrace= " + Logger.getStackTrace(t));
         
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Sends the method invoked message to the listener, if one has been set.
    * 
    * @param rval the return value from the method that was invoked.
    */
   protected void sendMessage(Object rval)
   {
      // if listener or callback invoker is set, send message back
      if(mListener != null || mCallbackMethodInvoker != null)
      {
         Object lockObject = mListener;
         if(lockObject == null)
         {
            lockObject = mCallbackMethodInvoker;
         }
         
         // lock on lock object (listener or callback method invoker) 
         synchronized(lockObject)
         {
            try
            {
               // populate message object
               mMessage.setMethodAgent(mAgent);
               mMessage.setMethodName(mMethodName);
               mMessage.setMethod(mMethod);
               mMessage.setMethodParams(mParams);
               mMessage.setMethodReturnValue(rval);
               
               // send message to listener or use callback method invoker
               if(mListener != null)
               {
                  mListener.methodInvoked(mMessage);
               }
               else
               {
                  // execute callback
                  mCallbackMethodInvoker.execute();
               }
            }
            catch(Throwable t)
            {
               String message = mMessage.getMessage();
               
               // if text message is blank, use method name
               if(mMessage.getMessage().equals(""))
               {
                  message = mMessage.getMethodName();
               }
               
               getLogger().error(
                  "an exception occurred while handling message: '" +
                  message + "'," +
                  "\nexception= " + t +
                  "\ncause= " + t.getCause() +
                  "\ntrace= " + Logger.getStackTrace(t));
               getLogger().debug(Logger.getStackTrace(t));
            }
         }
      }
   }

   /**
    * Gets the method to invoke.
    * 
    * @return the method to invoke.
    */
   protected Method getMethod()
   {
      if(mMethod == null)
      {
         mMethod = findMethod(mAgent, mMethodName, mParams);
      }
      
      return mMethod;
   }
   
   /**
    * Gets a method signature string.
    * 
    * @param methodName the method name.
    * @param params the parameters.
    * 
    * @return the signature string.
    */
   public static String getSignature(String methodName, Object[] params)
   {
      // build signature string
      String signature = methodName + "(";
      
      for(int i = 0; i < params.length; i++)
      {
         if(params[i] != null)
         {
            signature += params[i].getClass().getName();
         }
         else
         {
            signature += "null";
         }
         
         if(i < (params.length - 1))
         {
            signature += ", ";
         }
      }
      
      signature += ")";
      
      return signature;
   }
   
   /**
    * Gets a method signature string.
    * 
    * @param method the method.
    * 
    * @return the signature string.
    */
   public static String getSignature(Method method)
   {
      // build signature string
      String signature = "null";
      
      if(method != null)
      {
         signature = method.getName() + "(";
         
         Class[] types = method.getParameterTypes();
         for(int i = 0; i < types.length; i++)
         {
            signature += types[i].getName();
            
            if(i < (types.length - 1))
            {
               signature += ", ";
            }
         }
         
         signature += ")";
      }
      
      return signature;
   }
   
   /**
    * Gets the method class.
    * 
    * @param agent the agent.
    * 
    * @return the method class.
    */
   public static Class getMethodClass(Object agent)
   {
      Class methodClass = null;
      
      if(agent != null)
      {
         // get the method class: if the agent is a class -- if so, use it,
         // if not, use the agent's class
         if(agent instanceof Class)
         {
            methodClass = (Class)agent;
         }
         else
         {
            methodClass = agent.getClass();
         }
      }
      
      return methodClass;
   }
   
   /**
    * Gets the distance (in number of inherited classes) between
    * the passed base class and the passed derived class. 
    * 
    * @param baseClass the base class.
    * @param derivedClass the derivedClass.
    * 
    * @return the distance (in number of inherited classes) between
    *         the passed base class and the passed derived class.
    */
   public static int getInheritanceDistance(Class baseClass, Class derivedClass)
   {
      int rval = 0;
      
      if(derivedClass != null && baseClass != derivedClass)
      {
         rval = 1 + getInheritanceDistance(
            baseClass, derivedClass.getSuperclass());
      }
      
      return rval;
   }
   
   /**
    * Determines the distance (in number of inherited classes) between
    * parameter types and the passed parameters.
    * 
    * A return value of 0 indicates that the passed parameter types match
    * the classes of the passed parameters exactly without having to go up the
    * inheritance chain.
    * 
    * A return value of -1 indicates that the passed parameter types do
    * not match at all.
    * 
    * @param types the parameter types to check.
    * @param params the parameters to match.
    * 
    * @return the distance (in number of inherited classes) between
    *         method parameter types and the passed parameters as an
    *         integer between 0 and Integer.MAX_VALUE or -1 indicating
    *         that the parameter types do not match at all.
    */
   public static int getParameterDistance(Class[] types, Object[] params)
   {
      int rval = -1;
      
      // for determining if parameter types match
      boolean typesMatch = true;
      
      // for storing total parameter distance
      int distance = 0;
      
      // iterate through all parameters
      for(int i = 0; typesMatch && i < types.length; i++)
      {
         // get the parameter class
         Class paramClass = null;
         if(params[i] != null)
         {
            paramClass = params[i].getClass();
         }
         
         // determine if the parameter types match
         // ensure the parameter is an appropriate type
         if(!types[i].isInstance(params[i]))
         {
            // see if the type is a primative
            if(types[i].isPrimitive())
            {
               // FIXME: some primitives work that aren't exact
               // matches (i.e. an int passed as a long)
               
               if(paramClass != null)
               {
                  // determine if types match
                  String typePrimitive = "java.lang." + types[i].getName();
                  String paramPrimitive = paramClass.getName().toLowerCase();
                  typesMatch = paramPrimitive.equals(typePrimitive);
               }
               else
               {
                  // types do not match because param class is null
                  // and types[i] is a primative
                  typesMatch = false;
               }
            }
         }
         
         // if types match, determine parameter distance
         if(typesMatch)
         {
            // FIXME: some primitives work that aren't exact
            // matches (i.e. an int passed as a long)

            // if parameter type is a primitive then distance is zero,
            // otherwise we may need to add distance
            if(!types[i].isPrimitive())
            {
               // add the inheritance distance between the type class
               // and the parameter class
               distance += getInheritanceDistance(types[i], paramClass); 
            }
         }
      }
      
      // if types match, return distance
      if(typesMatch)
      {
         rval = distance;
      }
      
      return rval;
   }
   
   /**
    * Finds the method to invoke.
    * 
    * @param agent the object to invoke the method on.
    * @param methodName the name of the method to invoke.
    * @param params the parameters for the method.
    * 
    * @return the method to invoke or null if no such method exists.
    */
   public static Method findMethod(
      Object agent, String methodName, Object[] params)
   {
      Method rval = null;
      
      // get signature and agent name
      String signature = getSignature(methodName, params);
      Class methodClass = getMethodClass(agent);
      getLogger().debug(
         "searching for method: '" + signature +
         "' in class '" + methodClass.getName() + "'");
      
      // for storing the minimum parameter distance (in # of inherited classes)
      int minDistance = -1;
      
      // get methods
      Method[] methods = methodClass.getMethods();
      for(int i = 0; i < methods.length; i++)
      {
         // see if the method name matches
         if(methods[i].getName().equals(methodName))
         {
            // see if the parameter count matches
            Class[] types = methods[i].getParameterTypes();
            if(types.length == params.length)
            {
               // get the parameter distance
               int distance = getParameterDistance(types, params);
               
               // determine if the distance is the new minimum
               if(minDistance == -1 ||
                  (distance != -1 && distance < minDistance))
               {
                  // update new minimum distance and the method
                  minDistance = distance;
                  rval = methods[i];
               }
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Invokes the method and sends a message when the method completes,
    * if appropriate.
    */
   public void run()
   {
      // get the method to invoke
      Method method = getMethod();
      
      if(method != null)
      {
         // if the lock object is not null, synchronize on it
         if(mLockObject != null)
         {
            // if synchronizing the message handling, then lock sending
            // the message, otherwise lock only the method invocation.
            if(mLockMessageHandling)
            {
               // lock for method invocation and message handling
               synchronized(mLockObject)
               {
                  // invoke method and send message
                  sendMessage(invokeMethod(method, mAgent, mParams));
               }
            }
            else
            {
               Object rval = null;
               synchronized(mLockObject)
               {
                  rval = invokeMethod(method, mAgent, mParams);
               }
               
               // send message after releasing lock
               sendMessage(rval);
            }
         }
         else
         {
            // invoke method and send message
            sendMessage(invokeMethod(method, mAgent, mParams));
         }
      }
      else
      {
         // get signature and agent name
         String signature = getSignature(mMethodName, mParams);
         Class methodClass = getMethodClass(mAgent);
         getLogger().error(
            "could not invoke method: '" + signature +
            "' from class: '" + methodClass.getName() +
            "', method not recognized.");
      }
   }
   
   /**
    * Executes this MethodInvoker.
    */
   public void execute()
   {
      // foreground execute
      execute((MethodInvokerListener)null, null, null, false);
   }
   
   /**
    * Executes this MethodInvoker.
    * 
    * @param lockObject an object to lock on while invoking the method.
    */
   public void execute(Object lockObject)
   {
      // foreground execute
      execute((MethodInvokerListener)null, null, lockObject, false);
   }
   
   /**
    * Executes this MethodInvoker and reports back the passed
    * MethodInvokedMessage to the passed MethodInvokerListener when the
    * method completes. The passed message object will be populated
    * with information from the method that is invoked. A custom
    * message and object can be set in the message object before
    * passing it to this method.
    *
    * @param mil the MethodInvokerListener to send a message when the
    *            method completes.
    * @param mim the MethodInvokedMessage to populate and send to the
    *            passed MethodInvokerListener when the method completes.
    */
   public void execute(MethodInvokerListener mil, MethodInvokedMessage mim)
   {
      // foreground execute
      execute(mil, mim, null, false);
   }
   
   /**
    * Executes this MethodInvoker and reports back the passed
    * MethodInvokedMessage to the passed MethodInvokerListener when the
    * method completes. The passed message object will be populated
    * with information from the method that is invoked. A custom
    * message and object can be set in the message object before
    * passing it to this method.
    *
    * @param mil the MethodInvokerListener to send a message when the
    *            method completes.
    * @param mim the MethodInvokedMessage to populate and send to the
    *            passed MethodInvokerListener when the method completes.
    * @param lockObject an object to lock on while invoking the method.
    * @param lockMessageHandling true to continue to lock on the lock object
    *                            until the method invoker listener finishes
    *                            handling the method invoked message, false
    *                            to only lock during method invocation.
    */
   public void execute(MethodInvokerListener mil, MethodInvokedMessage mim,
                       Object lockObject, boolean lockMessageHandling)
   {
      // save listener and message
      mListener = mil;
      mMessage = mim;
      
      // if listener is not null, do not allow null message
      if(mListener != null && mMessage == null)
      {
         mMessage = new MethodInvokedMessage();
      }

      // set lock object and whether or not to lock message handling
      mLockObject = lockObject;
      mLockMessageHandling = lockMessageHandling;
      
      // run in this thread
      run();
   }
   
   /**
    * Executes this MethodInvoker and uses the passed MethodInvoker
    * to invoke a callback when the method completes. The passed message
    * object will be populated with information from the method that is
    * invoked. A custom message and object can be set in the message object
    * before passing it to this method.
    *
    * @param cbInvoker the MethodInvoker to invoke a callback with when
    *                  the method completes.
    * @param mim the MethodInvokedMessage to populate when the method completes.
    */
   public void execute(MethodInvoker cbInvoker, MethodInvokedMessage mim)
   {
      // foreground execute
      execute(cbInvoker, mim, null, false);
   }
   
   /**
    * Executes this MethodInvoker and uses the passed MethodInvoker
    * to invoke a callback when the method completes. The passed message
    * object will be populated with information from the method that is
    * invoked. A custom message and object can be set in the message object
    * before passing it to this method.
    *
    * @param cbInvoker the MethodInvoker to invoke a callback with when
    *                  the method completes.
    * @param mim the MethodInvokedMessage to populate when the method completes.
    * @param lockObject an object to lock on while invoking the method.
    * @param lockMessageHandling true to continue to lock on the lock object
    *                            until the method invoker listener finishes
    *                            handling the method invoked message, false
    *                            to only lock during method invocation.
    */
   public void execute(MethodInvoker cbInvoker, MethodInvokedMessage mim,
                       Object lockObject, boolean lockMessageHandling)
   {
      // save callback method invoker and message
      mCallbackMethodInvoker = cbInvoker;
      mMessage = mim;
      
      // if callback method invoker is not null, do not allow null message
      if(mCallbackMethodInvoker != null && mMessage == null)
      {
         mMessage = new MethodInvokedMessage();
      }

      // set lock object and whether or not to lock message handling
      mLockObject = lockObject;
      mLockMessageHandling = lockMessageHandling;
      
      // run in this thread
      run();      
   }
   
   /**
    * Executes this MethodInvoker in another thread.
    */
   public void backgroundExecute()
   {
      // background execute
      backgroundExecute((MethodInvokerListener)null, null, null, false);
   }
   
   /**
    * Executes this MethodInvoker in another thread.
    * 
    * @param lockObject an object to lock on while invoking the method.
    */
   public void backgroundExecute(Object lockObject)
   {
      // background execute
      backgroundExecute((MethodInvokerListener)null, null, lockObject, false);
   }   
   
   /**
    * Executes this MethodInvoker in another thread and reports back the passed
    * MethodInvokedMessage to the passed MethodInvokerListener when the
    * method completes. The passed message object will be populated
    * with information from the method that is invoked. A custom
    * message and object can be set in the message object before
    * passing it to this method.
    *
    * @param mil the MethodInvokerListener to send a message when the
    *            method completes.
    * @param mim the MethodInvokedMessage to populate and send to the
    *            passed MethodInvokerListener when the method completes.
    */
   public void backgroundExecute(MethodInvokerListener mil,
                                 MethodInvokedMessage mim)
   {
      // background execute
      backgroundExecute(mil, mim, null, false);
   }
   
   /**
    * Executes this MethodInvoker in another thread and reports back the passed
    * MethodInvokedMessage to the passed MethodInvokerListener when the
    * method completes. The passed message object will be populated
    * with information from the method that is invoked. A custom
    * message and object can be set in the message object before
    * passing it to this method.
    *
    * @param mil the MethodInvokerListener to send a message when the
    *            method completes.
    * @param mim the MethodInvokedMessage to populate and send to the
    *            passed MethodInvokerListener when the method completes.
    * @param lockObject an object to lock on while invoking the method.
    * @param lockMessageHandling true to continue to lock on the lock object
    *                            until the method invoker listener finishes
    *                            handling the method invoked message, false
    *                            to only lock during method invocation.
    */
   public void backgroundExecute(MethodInvokerListener mil,
                                 MethodInvokedMessage mim,
                                 Object lockObject, boolean lockMessageHandling)
   {
      // save listener and message
      mListener = mil;
      mMessage = mim;
      
      // if listener is not null, do not allow null message
      if(mListener != null && mMessage == null)
      {
         mMessage = new MethodInvokedMessage();
      }
      
      // set lock object and whether or not to lock message handling
      mLockObject = lockObject;
      mLockMessageHandling = lockMessageHandling;
      
      // start thread
      start();
   }
   
   /**
    * Executes this MethodInvoker in another thread and uses the passed
    * MethodInvoker to invoke a callback when the method completes. The
    * passed message object will be populated with information from the method
    * that is invoked. A custom message and object can be set in the message
    * object before passing it to this method.
    *
    * @param cbInvoker the MethodInvoker to invoke a callback with when
    *                  the method completes.
    * @param mim the MethodInvokedMessage to populate when the method completes.
    */
   public void backgroundExecute(MethodInvoker cbInvoker,
                                 MethodInvokedMessage mim)
   {
      // background execute
      backgroundExecute(cbInvoker, mim, null, false);      
   }
   
   /**
    * Executes this MethodInvoker in another thread and uses the passed
    * MethodInvoker to invoke a callback when the method completes. The
    * passed message object will be populated with information from the method
    * that is invoked. A custom message and object can be set in the message
    * object before passing it to this method.
    *
    * @param cbInvoker the MethodInvoker to invoke a callback with when
    *                  the method completes.
    * @param mim the MethodInvokedMessage to populate when the method completes.
    * @param lockObject an object to lock on while invoking the method.
    * @param lockMessageHandling true to continue to lock on the lock object
    *                            until the method invoker listener finishes
    *                            handling the method invoked message, false
    *                            to only lock during method invocation.
    */   
   public void backgroundExecute(MethodInvoker cbInvoker,
                                 MethodInvokedMessage mim,
                                 Object lockObject, boolean lockMessageHandling)
   {
      // save callback method invoker and message
      mCallbackMethodInvoker = cbInvoker;
      mMessage = mim;
      
      // if callback method invoker is not null, do not allow null message
      if(mCallbackMethodInvoker != null && mMessage == null)
      {
         mMessage = new MethodInvokedMessage();
      }
      
      // set lock object and whether or not to lock message handling
      mLockObject = lockObject;
      mLockMessageHandling = lockMessageHandling;
      
      // start thread
      start();
   }
   
   /**
    * Gets the logger.
    *
    * @return the logger.
    */
   public static Logger getLogger()
   {
      return LoggerManager.getLogger("dbutil");
   }
}
