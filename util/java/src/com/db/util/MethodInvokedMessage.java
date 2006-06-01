/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

/**
 * A MethodInvokedMessage object is an object that contains a text message
 * from an object that used a MethodInvoker to invoke a method. It also
 * contains the return value from the method that was invoked and an
 * optional custom message object.
 * 
 * A MethodInvokedMessage can be extended to contain other message information
 * if necessary.
 * 
 * @author Dave Longley
 */
public class MethodInvokedMessage
{
   /**
    * The agent of the method that was invoked (the object that the method
    * was invoked on).
    */
   protected Object mAgent;
   
   /**
    * The name of the method that was invoked.
    */
   protected String mMethodName;
   
   /**
    * The parameters of the method that was invoked.
    */
   protected Object[] mParams;
   
   /**
    * The return value from the method that was invoked.
    */
   protected Object mReturnValue;
   
   /**
    * Any exception that was thrown while the method was being invoked.
    */
   protected Throwable mException;
   
   /**
    * The text message from the object that used the MethodInvoker.
    */
   protected String mMessage;
   
   /**
    * A custom object to be set by the object that used the MethodInvoker.
    */
   protected Object mCustomObject;
   
   /**
    * Creates a new, empty message object.
    */
   public MethodInvokedMessage()
   {
      this("", null);
   }
   
   /**
    * Creates a new message object with the specified text message.
    * 
    * @param message the text message. 
    */
   public MethodInvokedMessage(String message)
   {
      this(message, null);
   }
   
   /**
    * Creates a new message object with the specified text message and custom
    * object.
    * 
    * @param message the text message.
    * @param customObject the custom object. 
    */
   public MethodInvokedMessage(String message, Object customObject)
   {
      mAgent = null;
      mMethodName = "";
      mParams = new Object[0];
      mReturnValue = null;
      mException = null;
      
      mMessage = message;
      mCustomObject = customObject;
   }
   
   /**
    * Sets the object that the method was invoked on. This value
    * will be set by the MethodInvoker that uses this message object.
    * 
    * @param agent the object that the method was invoked on.
    */
   public void setMethodAgent(Object agent)
   {
      mAgent = agent;
   }
   
   /**
    * Gets the object that the method was invoked on.
    * 
    * @return the object that the method was invoked on.
    */
   public Object getMethodAgent()
   {
      return mAgent;
   }
   
   /**
    * Sets the method name for the method that was invoked. This value
    * will be set by the MethodInvoker that uses this message object.
    * 
    * @param methodName the method name for the method that was invoked.
    */
   public void setMethodName(String methodName)
   {
      mMethodName = methodName;
   }
   
   /**
    * Gets the method name for the method that was invoked.
    * 
    * @return the method name for the method that was invoked.
    */
   public String getMethodName()
   {
      return mMethodName;
   }
   
   /**
    * Sets the parameters for the method that was invoked. This value
    * will be set by the MethodInvoker that uses this message object.
    * 
    * @param params the parameters for the method that was invoked.
    */
   public void setMethodParams(Object[] params)
   {
      if(params == null)
      {
         mParams = new Object[0];
      }
      else
      {
         mParams = params;
      }
   }
   
   /**
    * Gets the parameters for the method that was invoked.
    * 
    * @return the parameters for the method that was invoked.
    */
   public Object[] getMethodParams()
   {
      return mParams;
   }
   
   /**
    * Sets the return value for the method that was invoked. This value
    * will be set by the MethodInvoker that uses this message object.
    * 
    * @param rval the return value for the method that was invoked.
    */
   public void setMethodReturnValue(Object rval)
   {
      mReturnValue = rval;
   }
   
   /**
    * Gets the return value for the method that was invoked.
    * 
    * @return the return value for the method that was invoked.
    */
   public Object getMethodReturnValue()
   {
      return mReturnValue;
   }
   
   /**
    * Sets the exception that was thrown while the method was being
    * invoked, if any. This value will be set by the MethodInvoker that
    * uses this message object.
    * 
    * @param exception the exception for the method that was invoked.
    */
   public void setMethodException(Throwable exception)
   {
      mException = exception;
   }
   
   /**
    * Gets the exception that was thrown while the method was being
    * invoked, if any.
    * 
    * @return the exception that was thrown while the method was being
    *         invoked or null if no exception was thrown. 
    */
   public Throwable getMethodException()
   {
      return mException;
   }
   
   /**
    * Sets the text message for this message object.
    * 
    * @param message the text message for this message object.
    */
   public void setMessage(String message)
   {
      mMessage = message;
   }
   
   /**
    * Gets the text message from this message object.
    * 
    * @return the text message from this message object.
    */
   public String getMessage()
   {
      return mMessage;
   }
   
   /**
    * Sets the custom object for this message object.
    * 
    * @param customObject the custom object for this message object.
    */
   public void setCustomObject(Object customObject)
   {
      mCustomObject = customObject;
   }
   
   /**
    * Gets the custom object from this message object.
    * 
    * @return the custom object from this message object.
    */
   public Object getCustomObject()
   {
      return mCustomObject;
   }
}
