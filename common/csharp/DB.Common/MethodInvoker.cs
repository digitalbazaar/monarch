/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Reflection;
using System.Threading;

using DB.Common.Logging;

namespace DB.Common
{
   /// <summary>
   /// Called when a MethodInvoker finishes invoking its method.
   /// </summary> 
   /// 
   /// <param name="mim">a message with information about the method that
   /// was invoked.</param>
   public delegate void MethodInvokedDelegate(MethodInvokedMessage mim);
   
   /// <summary>
   /// This object is used to invoke a method, optionally, report a message
   /// back when the method completes. The method can be invoked in another
   /// thread or on the current one.
   /// </summary>
   ///
   /// <author>Dave Longley</author>
   public class MethodInvoker
   {
      /// <summary>
      /// The object with the method to invoke.
      /// </summary>
      protected object mAgent;
      
      /// <summary>
      /// The name of the method to invoke.
      /// </summary>
      protected string mMethodName;
      
      /// <summary>
      /// A delegate to call (rather than using a provided agent and method
      /// name).
      /// </summary>
      protected Delegate mDelegate;
      
      /// <summary>
      /// The method parameters.
      /// </summary>
      protected object[] mParams;
      
      /// <summary>
      /// A custom lock object to synchronize on.
      /// </summary>
      protected object mLockObject;
   
      /// <summary>
      /// True if the lock object should remain locked until the method invoked
      /// message has been handled, false if it should unlock immediately after
      /// the method has been invoked.
      /// </summary>
      protected bool mLockMessageHandling;
      
      /// <summary>
      /// A MethodInvokedMessage to send to a MethodInvokerListener after the
      /// method has been invoked.
      /// </summary>
      protected MethodInvokedMessage mMessage;
      
      /// <summary>
      /// The MethodInvokedDelegate to call once the method completes.
      /// </summary>
      protected MethodInvokedDelegate mMethodInvokedDelegate;
      
      /// <summary>
      /// Creates a new MethodInvoker thread.
      /// </summary>
      ///
      /// <param name="agent">the object that the method will be invoked
      /// on.</param>
      /// <param name="methodName">the name of the method to invoke.</param>
      /// <param name="parameters">the parameters for the method.</param>
      public MethodInvoker(object agent, string methodName,
                           params object[] parameters)
      {
         mAgent = agent;
         mMethodName = methodName;
         mParams = parameters;
         
         if(mParams == null)
         {
            mParams = new object[0];
         }
         
         mLockObject = null;
         mLockMessageHandling = false;
         
         mMessage = null;
         mMethodInvokedDelegate = null;
      }

      /// <summary>
      /// Creates a new MethodInvoker thread.
      /// </summary>
      ///
      /// <param name="d">the delegate that will be invoked.</param>
      /// <param name="parameters">the parameters for the delegate.</param>
      public MethodInvoker(Delegate d, params object[] parameters)
      {
         mAgent = d.Target;
         mMethodName = d.Method.Name;
         mDelegate = d;
         mParams = parameters;
         
         if(mParams == null)
         {
            mParams = new object[0];
         }
         
         mLockObject = null;
         mLockMessageHandling = false;
         
         mMessage = null;
         mMethodInvokedDelegate = null;
      }
      
      /// <summary>
      /// Finds the method info for the method to invoke.
      /// </summary>
      /// 
      /// <param name="agent">the object to invoke the method on.</param>
      /// <param name="methodName">the name of the method to invoke.</param>
      /// <param name="parameters">the parameters for the method.</param>
      /// 
      /// <returns>the method info for the method to invoke or null if
      /// no appropriate method info could be found.</returns>
      protected virtual MethodInfo FindMethodInfo(object agent,
                                                  string methodName,
                                                  object[] parameters)
      {
         MethodInfo rval = null;
         
         // get all public methods
         MethodInfo[] methodInfos = new MethodInfo[0];
         if(agent is Type)
         {
            methodInfos = ((Type)agent).GetMethods(BindingFlags.Public);
         }
         else
         {
            methodInfos = agent.GetType().GetMethods(BindingFlags.Instance |
                                                     BindingFlags.Public);
         }
         
         foreach(MethodInfo mi in methodInfos)
         {
            if(mi.Name == methodName)
            {
               ParameterInfo[] parameterInfos = mi.GetParameters();
               if(parameterInfos.Length == parameters.Length)
               {
                  bool typesMatch = true;
                  for(int i = 0; i < parameterInfos.Length; i++)
                  {
                     if(!parameterInfos[i].ParameterType.
                         IsInstanceOfType(parameters[i]))
                     {
                        typesMatch = false;
                        break;
                     }
                  }
                  
                  // if the parameter types match, proceed
                  if(typesMatch)
                  {
                     // method info has been found
                     rval = mi;
                     break;
                  }
               }
            }
         }
         
         return rval;
      }

      /// <summary>
      /// Invokes the method described by the passed method info or
      /// wrapped by an internally set delegate.
      /// </summary>
      ///
      /// <param name="mi">the method info for the method to
      /// invoke, or null if a delegate will be used to invoke
      /// the method.</param>
      /// 
      /// <returns>the return value from the method.</returns>
      protected virtual object InvokeMethod(MethodInfo mi)
      {
         object rval = null;
         
         try
         {
            // invoke method
            if(mDelegate != null)
            {
               rval = mDelegate.DynamicInvoke(mParams);
            }
            else
            {
               rval = mi.Invoke(mAgent, mParams);
            }
         }
         catch(Exception e)
         {
            if(mMessage != null)
            {
               mMessage.MethodException = e;
            }
            
            Logger.Error("could not invoke method: '" +
                         mMethodName +
                         "', an exception occurred!," +
                         "exception=" + e.GetType().Name + ": " + e.Message);
            Logger.Debug(e.ToString());
         }
         
         return rval;
      }

      /// <summary>
      /// Sends the message to the method invoked delegate, if one has
      /// been set.
      /// </summary>
      ///
      /// <param name="rval">the return value from the method that
      /// was invoked.</param>
      protected virtual void SendMessage(object rval)
      {
         // if method invoked delegate is set, send message back
         if(mMethodInvokedDelegate != null)
         {
            // lock on delegate's target
            lock(mMethodInvokedDelegate.Target)
            {
               try
               {
                  // populate message object
                  mMessage.MethodAgent = mAgent;
                  mMessage.MethodName = mMethodName;
                  mMessage.MethodParams = mParams;
                  mMessage.MethodReturnValue = rval;
                  
                  // call delegate
                  mMethodInvokedDelegate(mMessage);
               }
               catch(Exception e)
               {
                  string message = mMessage.Message;
                  
                  // if text message is blank, use method name
                  if(mMessage.Message == string.Empty)
                  {
                     message = mMessage.MethodName;
                  }
                  
                  Logger.Error("failed to handle message: '" +
                               message + "', an exception occurred!," +
                               "exception=" + e.GetType().Name + ": " +
                               e.Message);
                  Logger.Debug(e.ToString());
               }
            }
         }
      }
      
      /// <summary>
      /// Invokes the method and sends a message when the method completes,
      /// if appropriate.
      /// </summary>
      protected virtual void Run()
      {
         Logger.Debug("attempting to invoke method: '" + mMethodName + "'");
         
         MethodInfo mi = null;
         if(mDelegate == null)
         {
            // find the method info for the method to invoke
            mi = FindMethodInfo(mAgent, mMethodName, mParams);
         }
            
         if(mi != null || mDelegate != null)
         {
            // if the lock object is not null, lock on it
            if(mLockObject != null)
            {
               // if locking the message handling, then lock sending
               // the message, otherwise lock only the method invocation.
               if(mLockMessageHandling)
               {
                  // lock for method invocation and message handling
                  lock(mLockObject)
                  {
                     // invoke method
                     object rval = InvokeMethod(mi);
                     
                     // send message
                     SendMessage(rval);
                  }
               }
               else
               {
                  object rval = null;
                  lock(mLockObject)
                  {
                     // invoke method
                     rval = InvokeMethod(mi);
                  }
                  
                  // send message after releasing lock
                  SendMessage(rval);
               }
            }
            else
            {
               // invoke method
               object rval = InvokeMethod(mi);
               
               // send message
               SendMessage(rval);
            }
         }
         else
         {
            Logger.Error("could not invoke method: '" + mMethodName +
                         "', method not recognized.");
         }
      }
      
      /// <summary>
      /// Executes this MethodInvoker.
      /// </summary>
      public virtual void Execute()
      {
         // execute in foreground
         Execute(null, null, null, false);
      }
      
      /// <summary>
      /// Executes this MethodInvoker.
      /// </summary>
      /// 
      /// <param name="lockObject">an object to lock on while invoking the
      /// method.</param>
      public virtual void Execute(object lockObject)
      {
         // execute in foreground
         Execute(null, null, lockObject, false);
      }
      
      /// <summary>
      /// Executes this MethodInvoker and reports back the passed
      /// MethodInvokedMessage to the passed MethodInvokedDelegate when the
      /// method completes.
      /// </summary>
      ///
      /// <param name="mid">the MethodInvokedDelegate to send a message when
      /// the method completes.</param>
      /// <param name="mim">the MethodInvokedMessage to populate and send to
      /// the passed MethodInvokedDelegate when the method completes.</param>
      public virtual void Execute(MethodInvokedDelegate mid,
                                  MethodInvokedMessage mim)
      {
         // execute in foreground
         Execute(mid, mim, null, false);
      }

      /// <summary>
      /// Executes this MethodInvoker and reports back the passed
      /// MethodInvokedMessage to the passed MethodInvokedDelegate when the
      /// method completes.
      /// </summary>
      ///
      /// <param name="mid">the MethodInvokedDelegate to send a message when
      /// the method completes.</param>
      /// <param name="mim">the MethodInvokedMessage to populate and send to
      /// the passed MethodInvokedDelegate when the method completes.</param>
      /// <param name="lockObject">an object to lock on while invoking the
      /// method.</param>
      /// <param name="lockMessageHandling">true to continue to lock on the
      /// lock object until the method invoker listener finishes handling the
      /// method invoked message, false to only lock during method invocation.
      /// </param>
      public virtual void Execute(MethodInvokedDelegate mid,
                                  MethodInvokedMessage mim,
                                  object lockObject, bool lockMessageHandling)
      {
         // save delegate and message
         mMethodInvokedDelegate = mid;
         mMessage = mim;
         
         // if delegate is not null, do not allow null message
         if(mMethodInvokedDelegate != null && mMessage == null)
         {
            mMessage = new MethodInvokedMessage();
         }
         
         // set lock object and whether or not to lock message handling
         mLockObject = lockObject;
         mLockMessageHandling = lockMessageHandling;
         
         // run in this thread
         Run();
      }
      
      /// <summary>
      /// Executes this MethodInvoker in another thread.
      /// </summary>
      public virtual void BackgroundExecute()
      {
         // execute in background
         BackgroundExecute(null, null, null, false);
      }
      
      /// <summary>
      /// Executes this MethodInvoker in another thread.
      /// </summary>
      /// 
      /// <param name="lockObject">an object to lock on while invoking the
      /// method.</param>
      public virtual void BackgroundExecute(object lockObject)
      {
         // execute in background
         BackgroundExecute(null, null, lockObject, false);
      }
      
      /// <summary>
      /// Executes this MethodInvoker in another thread and reports back the
      /// passed MethodInvokedMessage to the passed MethodInvokedDelegate when
      /// the method completes.
      /// </summary>
      ///
      /// <param name="mid">the MethodInvokedDelegate to send a message when
      /// the method completes.</param>
      /// <param name="mim">the MethodInvokedMessage to populate and send to
      /// the passed MethodInvokedDelegate when the method completes.</param>
      public virtual void BackgroundExecute(MethodInvokedDelegate mid,
                                            MethodInvokedMessage mim)
      {
         // execute in background
         BackgroundExecute(mid, mim, null, false);
      }

      /// <summary>
      /// Executes this MethodInvoker in another thread and reports back the
      /// passed MethodInvokedMessage to the passed MethodInvokedDelegate when
      /// the method completes.
      /// </summary>
      ///
      /// <param name="mid">the MethodInvokedDelegate to send a message when
      /// the method completes.</param>
      /// <param name="mim">the MethodInvokedMessage to populate and send to
      /// the passed MethodInvokedDelegate when the method completes.</param>
      /// <param name="lockObject">an object to lock on while invoking the
      /// method.</param>
      /// <param name="lockMessageHandling">true to continue to lock on the
      /// lock object until the method invoker listener finishes handling the
      /// method invoked message, false to only lock during method invocation.
      /// </param>
      public virtual void BackgroundExecute(MethodInvokedDelegate mid,
                                            MethodInvokedMessage mim,
                                            object lockObject,
                                            bool lockMessageHandling)
      {
         // save delegate and message
         mMethodInvokedDelegate = mid;
         mMessage = mim;
         
         // if delegate is not null, do not allow null message
         if(mMethodInvokedDelegate != null && mMessage == null)
         {
            mMessage = new MethodInvokedMessage();
         }
         
         // set lock object and whether or not to lock message handling
         mLockObject = lockObject;
         mLockMessageHandling = lockMessageHandling;
         
         // execute in another thread
         Thread t = new Thread(new ThreadStart(Run));
         t.Start();
      }
      
      /// <summary>
      /// Gets the logger.
      /// </summary>
      public virtual Logger Logger
      {
         get
         {
            return LoggerManager.GetLogger("dbcommon");
         }
      }
   }
}
