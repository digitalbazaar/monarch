/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

/**
 * A SoapFaultException is an exception that can be thrown when a
 * SOAP fault occurs. 
 * 
 * @author Dave Longley
 */
public class SoapFaultException extends Exception
{
   /**
    * The soap message that contains the soap fault.
    */
   protected SoapMessage mSoapMessage;
   
   /**
    * Creates a new SoapFaultException with null as its detail message.
    * 
    * The cause is not initialized, and may subsequently be initialized by a
    * call to initCause().
    * 
    * @param sm the soap message that contains the soap fault.
    */
   public SoapFaultException(SoapMessage sm)
   {
      this(sm, null, null);
   }

   /**
    * Constructs a new SoapFaultException with the specified detail message. 
    * 
    * The cause is not initialized, and may subsequently be initialized by
    * a call to initCause().
    *
    * @param sm the soap message that contains the soap fault.
    * @param message the detail message.
    * 
    */
   public SoapFaultException(SoapMessage sm, String message)
   {
      this(sm, message, null);
   }
   
   /**
    * Constructs a new SoapFaultException with the specified SoapMessage,
    * and cause. The detail message will be null if the cause is null,
    * otherwise it will be the string representation of the cause.
    *
    * @param sm the soap message that contains the soap fault.
    * @param cause the cause (null is permitted and indicates that the
    *              cause is nonexistent or unknown).
    */
   public SoapFaultException(SoapMessage sm, Throwable cause)
   {
      this(sm, null, cause);
   }

   /**
    * Constructs a new exception with the specified detail message and
    * cause.
    *
    * @param sm the soap message that contains the soap fault.
    * @param message the detail message.
    * @param cause the cause (null is permitted and indicates that the
    *              cause is nonexistent or unknown).
    */
   public SoapFaultException(SoapMessage sm, String message, Throwable cause)
   {
      // initialize message and cause
      super(message, cause);
      
      // store the soap message
      mSoapMessage = sm;
   }
   
   /**
    * Gets the soap message containing the soap fault.
    * 
    * @return the soap message containing the soap fault.
    */
   public SoapMessage getSoapMessage()
   {
      return mSoapMessage;
   }
}
