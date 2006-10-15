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
    * The soap fault.
    */
   protected SoapFault mSoapFault;
   
   /**
    * Creates a new SoapFaultException with the SOAP Fault string as its
    * detail message.
    * 
    * The cause is not initialized, and may subsequently be initialized by a
    * call to initCause().
    * 
    * @param fault the soap fault.
    */
   public SoapFaultException(SoapFault fault)
   {
      this(fault, fault.getFaultString(), null);
   }

   /**
    * Constructs a new SoapFaultException with the specified detail message. 
    * 
    * The cause is not initialized, and may subsequently be initialized by
    * a call to initCause().
    *
    * @param fault the soap fault.
    * @param message the detail message.
    * 
    */
   public SoapFaultException(SoapFault fault, String message)
   {
      this(fault, message, null);
   }
   
   /**
    * Constructs a new SoapFaultException with the specified SoapMessage,
    * and cause. The detail message will be null if the cause is null,
    * otherwise it will be the string representation of the cause.
    *
    * @param fault the soap fault.
    * @param cause the cause (null is permitted and indicates that the
    *              cause is nonexistent or unknown).
    */
   public SoapFaultException(SoapFault fault, Throwable cause)
   {
      this(fault, fault.getFaultString(), cause);
   }

   /**
    * Constructs a new exception with the specified detail message and
    * cause.
    *
    * @param fault the soap fault.
    * @param message the detail message.
    * @param cause the cause (null is permitted and indicates that the
    *              cause is nonexistent or unknown).
    */
   public SoapFaultException(SoapFault fault, String message, Throwable cause)
   {
      // initialize message and cause
      super(message, cause);
      
      // store the soap fault
      mSoapFault = fault;
   }
   
   /**
    * Gets the soap fault.
    * 
    * @return the soap fault.
    */
   public SoapFault getSoapFault()
   {
      return mSoapFault;
   }
}
