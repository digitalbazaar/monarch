/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

/**
 * A UPnPErrorException is an exception that can be thrown when a
 * UPnPError occurs. 
 * 
 * @author Dave Longley
 */
public class UPnPErrorException extends Exception
{
   /**
    * The UPnPError that occurred.
    */
   protected UPnPError mError;
   
   /**
    * Creates a new UPnPErrorException with the given UPnPError.
    * 
    * The cause is not initialized, and may subsequently be initialized by a
    * call to initCause().
    * 
    * @param error the UPnPError that occurred.
    */
   public UPnPErrorException(UPnPError error)
   {
      this(error, null, null);
   }

   /**
    * Constructs a new UPnPErrorException with the specified detail message. 
    * 
    * The cause is not initialized, and may subsequently be initialized by
    * a call to initCause().
    *
    * @param error the UPnPError that occurred.
    * @param message the detail message.
    * 
    */
   public UPnPErrorException(UPnPError error, String message)
   {
      this(error, message, null);
   }
   
   /**
    * Constructs a new UPnPErrorException with the specified UPnPError,
    * and cause. The detail message will be null if the cause is null,
    * otherwise it will be the string representation of the cause.
    *
    * @param error the UPnPError that occurred.
    * @param cause the cause (null is permitted and indicates that the
    *              cause is nonexistent or unknown).
    */
   public UPnPErrorException(UPnPError error, Throwable cause)
   {
      this(error, null, cause);
   }

   /**
    * Constructs a new UPnPErrorException with the specified detail message and
    * cause.
    *
    * @param error the UPnPError that occurred.
    * @param message the detail message.
    * @param cause the cause (null is permitted and indicates that the
    *              cause is nonexistent or unknown).
    */
   public UPnPErrorException(UPnPError error, String message, Throwable cause)
   {
      // initialize message and cause
      super(message, cause);
      
      // store the UPnPError
      mError = error;
   }
   
   /**
    * Gets the UPnPError that occurred.
    * 
    * @return the UPnPError that occurred.
    */
   public UPnPError getUPnPError()
   {
      return mError;
   }
}
