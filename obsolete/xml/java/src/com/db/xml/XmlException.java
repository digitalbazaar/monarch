/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

/**
 * An XmlException is an exception that can be thrown when an error occurs
 * while parsing or creating XML.
 * 
 * @author Dave Longley
 */
public class XmlException extends Exception
{
   /**
    * Constructs a new XmlException with the specified detail message. 
    * 
    * The cause is not initialized, and may subsequently be initialized by
    * a call to initCause().
    *
    * @param message the detail message.
    */
   public XmlException(String message)
   {
      this(message, null);
   }
   
   /**
    * Constructs a new XmlException with the specified detail message, and
    * cause.
    *
    * @param message the detail message.
    * @param cause the cause (null is permitted and indicates that the
    *              cause is nonexistent or unknown).
    */
   public XmlException(String message, Throwable cause)
   {
      // initialize message and cause
      super(message, cause);
   }
}
