/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

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
    * Creates a new MethodInvoker thread.
    * 
    * @param agent the object that the method will be invoked on.
    * @param methodName the name of the method to invoke.
    * @param params the parameters for the method.
    */
   public MethodInvoker(Object agent, String methodName, Object[] params)
   {
      mAgent = agent;
      mMethodName = methodName;
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
   }
   
   /**
    * Finds the method to invoke.
    * 
    * @param agent the object to invoke the method on.
    * @param methodName the name of the method to invoke.
    * @param params the parameters for the method.
    * @return the method to invoke or null if no such method exists.
    */
   protected Method findMethod(Object agent, String methodName, Object[] params)
   {
      Method rval = null;
      
      // get the methods for the agent, if the agent is a class obtain
      // then directly, if not, then get the class of the agent and
      // then obtain them
      Method[] methods = new Method[0];
      if(agent instanceof Class)
      {
         methods = ((Class)agent).getMethods();
      }
      else
      {
         methods = agent.getClass().getMethods();
      }
      
      for(int i = 0; i < methods.length; i++)
      {
         Method m = methods[i];
         if(m.getName().equals(methodName))
         {
            Class[] types = m.getParameterTypes();
            int numParams = types.length;
            if(params.length == numParams)
            {
               boolean typesMatch = true;
               for(int n = 0; n < numParams; n++)
               {
                  // ensure the parameter is the appropriate type
                  if(!types[n].isInstance(params[n]))
                  {
                     // assume failure unless a primitive is being used
                     typesMatch = false;
                     
                     if(types[n].isPrimitive())
                     {
                        // determine if the param type matches the primitive
                        String paramClass =
                           params[n].getClass().getName().toLowerCase();
                        String primitive =
                           "java.lang." + types[n].getName();
                        
                        typesMatch = paramClass.equals(primitive);
                     }
                     
                     // break out if types do not match
                     if(!typesMatch)
                     {
                        break;
                     }
                  }
               }
               
               // if the parameter types match, proceed
               if(typesMatch)
               {
                  // method found
                  rval = m;
                  break;
               }
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Invokes the method.
    * 
    * @param method the method to invoke.
    * @param agent the object to invoke the method on.
    * @param params the parameters for the method.
    * @return the return value from the method.
    */
   protected Object invokeMethod(Method method, Object agent, Object[] params) 
   {
      Object rval = null;
      
      try
      {
         // invoke method
         rval = method.invoke(agent, params);
      }
      catch(Throwable t)
      {
         if(mMessage != null)
         {
            mMessage.setMethodException(t);
         }
         
         String agentName = "null";
         if(agent != null)
         {
            if(agent instanceof Class)
            {
               agentName = ((Class)agent).getName();
            }
            else
            {
               agentName = agent.getClass().getName();
            }
         }
         
         getLogger().error(
            "could not invoke method: '" + mMethodName +
            "' on agent: '" + agentName + "', " +
            "an exception occurred!, exception=" + t.toString());
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
               
               getLogger().error("failed to handle message: '" +
                                 message + "', an exception occurred!," +
                                 "exception=" + t.toString());
               getLogger().debug(Logger.getStackTrace(t));
            }
         }
      }
   }
   
   /**
    * Invokes the method and sends a message when the method completes,
    * if appropriate.
    */
   public void run()
   {
      // build signature string
      String signature = mMethodName + "(";
      for(int i = 0; i < mParams.length; i++)
      {
         if(mParams[i] != null)
         {
            signature += mParams[i].getClass().getName();
         }
         else
         {
            signature += "null";
         }
         
         if(i < (mParams.length - 1))
         {
            signature += ", ";
         }
      }
      signature += ")";
      
      getLogger().debug("attempting to invoke method: '" + signature + "'");
      
      // find the method
      Method method = findMethod(mAgent, mMethodName, mParams);
      
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
         String agentName = "null";
         if(mAgent != null)
         {
            if(mAgent instanceof Class)
            {
               agentName = ((Class)mAgent).getName();
            }
            else
            {
               agentName = mAgent.getClass().getName();
            }
         }
         
         getLogger().error(
            "could not invoke method: '" + signature +
            "' on agent: '" + agentName + "', " + "method not recognized.");
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
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbcommon");
   }
}
