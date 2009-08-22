/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;

namespace DB.Common
{
   /// <summary>
   /// A MethodInvokedMessage object is an object that contains a text message
   /// from an object that used a MethodInvoker to invoke a method. It also
   /// contains the return value from the method that was invoked and an
   /// optional custom message object.
   /// </summary>
   ///
   /// A MethodInvokedMessage can be extended to contain other message
   /// information if necessary.
   ///
   /// <author>Dave Longley</author>
   public class MethodInvokedMessage
   {
      /// <summary>
      /// The agent of the method that was invoked (the object that the method
      /// was invoked on).
      /// </summary>
      protected object mAgent;
      
      /// <summary>
      /// The name of the method that was invoked.
      /// </summary>
      protected string mMethodName;
      
      /// <summary>
      /// The parameters of the method that was invoked.
      /// </summary>
      protected object[] mParams;
      
      /// <summary>
      /// The return value from the method that was invoked.
      /// </summary>
      protected object mReturnValue;
      
      /// <summary>
      /// Any exception that was thrown while trying to invoke the
      /// method.
      /// </summary>
      protected Exception mException;
      
      /// <summary>
      /// The text message from the object that used the MethodInvoker.
      /// </summary>
      protected string mMessage;
      
      /// <summary>
      /// A custom object to be set by the object that used the MethodInvoker.
      /// </summary>
      protected object mCustomObject;
      
      /// <summary>
      /// Creates a new, empty message object.
      /// </summary>
      public MethodInvokedMessage() : this(string.Empty, null)
      {
      }
      
      /// <summary>
      /// Creates a new message object with the specified text message.
      /// </summary>
      ///
      /// <param name="message">the text message.</param>
      public MethodInvokedMessage(string message) : this(message, null)
      {
      }
      
      /// <summary>
      /// Creates a new message object with the specified text message and 
      /// custom object.
      /// </summary>
      ///
      /// <param name="message">the text message.</param>
      /// <param name="customObject">the custom object.</param>
      public MethodInvokedMessage(string message, object customObject)
      {
         mAgent = null;
         mMethodName = string.Empty;
         mParams = new object[0];
         mReturnValue = null;
         mException = null;
         
         mMessage = message;
         mCustomObject = customObject;
      }
      
      /// <summary>
      /// Gets/Sets the object that the method was invoked on. This value
      /// will be set by the MethodInvoker that uses this message object.
      /// </summary>
      public virtual object MethodAgent
      {
         get
         {
            return mAgent;
         }
         
         set
         {
            mAgent = value;
         }
      }
      
      /// <summary>
      /// Gets/Sets the method name for the method that was invoked. This value
      /// will be set by the MethodInvoker that uses this message object.
      /// </summary>
      public virtual string MethodName
      {
         get
         {
            return mMethodName;
         }
         
         set
         {
            mMethodName = value;
         }
      }
      
      /// <summary>
      /// Gets/Sets the parameters for the method that was invoked. This value
      /// will be set by the MethodInvoker that uses this message object.
      /// </summary>
      public virtual object[] MethodParams
      {
         get
         {
            return mParams;
         }
         
         set
         {
            if(value == null)
            {
               mParams = new object[0];
            }
            else
            {
               mParams = value;
            }
         }
      }
      
      /// <summary>
      /// Gets/Sets the return value for the method that was invoked. This
      /// value will be set by the MethodInvoker that uses this message object.
      /// </summary>
      public virtual object MethodReturnValue
      {
         get
         {
            return mReturnValue;
         }
         
         set
         {
            mReturnValue = value;
         }
      }
      
      /// <summary>
      /// Gets/Sets any exception that was thrown while the method was
      /// being invoked. This value will be set by the MethodInvoker that
      /// uses this message object.
      /// </summary>
      public virtual Exception MethodException
      {
         get
         {
            return mException;
         }
         
         set
         {
            mException = value;
         }
      }
      
      /// <summary>
      /// Gets/Sets the text message for this message object.
      /// </summary>
      public virtual string Message
      {
         get
         {
            return mMessage;
         }
         
         set
         {
            mMessage = value;
         }
      }
      
      /// <summary>
      /// Gets/Sets the custom object for this message object.
      /// </summary>
      public virtual object CustomObject
      {
         get
         {
            return mCustomObject;
         }
         
         set
         {
            mCustomObject = value;
         }
      }
   }
}
