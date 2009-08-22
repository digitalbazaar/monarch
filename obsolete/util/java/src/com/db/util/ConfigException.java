/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

/**
 * A ConfigException is an exception that can be thrown when a configuration
 * error occurs. 
 * 
 * @author Dave Longley
 */
public class ConfigException extends Exception
{
   /**
    * The configuration options.
    */
   protected ConfigOptions mConfig;
   
   /**
    * Constructs a new ConfigException with the specified ConfigOptions an
    * detail message. 
    * 
    * The cause is not initialized, and may subsequently be initialized by
    * a call to initCause().
    *
    * @param config the ConfigOptions.
    * @param message the detail message.
    */
   public ConfigException(ConfigOptions config, String message)
   {
      this(config, message, null);
   }
   
   /**
    * Constructs a new ConfigException with the specified ConfigOptions,
    * detail message, and cause.
    *
    * @param config the ConfigOptions.
    * @param message the detail message.
    * @param cause the cause (null is permitted and indicates that the
    *              cause is nonexistent or unknown).
    */
   public ConfigException(ConfigOptions config, String message, Throwable cause)
   {
      // initialize message and cause
      super(message, cause);
      
      // store the configuration
      mConfig = config;
   }
   
   /**
    * Gets the configuration.
    * 
    * @return the configuration.
    */
   public ConfigOptions getConfig()
   {
      return mConfig;
   }
}
