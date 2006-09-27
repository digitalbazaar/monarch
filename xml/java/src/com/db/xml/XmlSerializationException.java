/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

/**
 * An XmlSerializationException is an exception that can be thrown during
 * xml serialization/deserialization.
 * 
 * @author Dave Longley
 */
public class XmlSerializationException extends Exception
{
   /**
    * Creates a new XmlSerializationException.
    * 
    * The cause is not initialized, and may subsequently be initialized by a
    * call to initCause().
    */
   public XmlSerializationException()
   {
      this(null, null);
   }

   /**
    * Constructs a new XmlSerializationException with the specified detail
    * message. 
    * 
    * The cause is not initialized, and may subsequently be initialized by
    * a call to initCause().
    *
    * @param message the detail message.
    * 
    */
   public XmlSerializationException(String message)
   {
      this(message, null);
   }
   
   /**
    * Constructs a new XmlSerializationException with the detail message
    * and cause. The detail message will be null if the cause is null,
    * otherwise it will be the string representation of the cause.
    *
    * @param message the detail message.
    * @param cause the cause (null is permitted and indicates that the
    *              cause is nonexistent or unknown).
    */
   public XmlSerializationException(String message, Throwable cause)
   {
      // initialize message and cause
      super(message, cause);
   }
}
