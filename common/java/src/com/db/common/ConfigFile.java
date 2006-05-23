/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import com.db.logging.Logger;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.Iterator;
import java.util.Properties;

/**
 * A class for managing a configuration file.
 * 
 * @author Dave Longley
 */
public class ConfigFile extends ConfigOptions
{
   /**
    * The name of the config file.
    */
   protected String mFilename;
   
   /**
    * A header to write in the file.
    */
   protected String mHeader;
   
   /**
    * Creates a new config file.
    * 
    * @param filename the name of the config file.
    */
   public ConfigFile(String filename)
   {
      mFilename = filename;
      mHeader = null;
   }
   
   /**
    * Loads the configuration settings from a file.
    *
    * @return true if successful, false if not.
    */
   public synchronized boolean read()
   {
      boolean rval = false;
      
      mProperties = new Properties();
   
      try
      {
         // read the configuration from a file
         mProperties.load(new FileInputStream(mFilename));
         rval = true;
      }
      catch(Throwable t)
      {
         getLogger().error(
               "Could not read config file: \"" + mFilename + "\"");
         getLogger().debug(Logger.getStackTrace(t));
      }

      return rval;
   }
   
   /**
    * Writes the configuration settings to a file.
    *
    * @return true if successful, false if not.
    */
   public boolean write()
   {
      return write(mHeader);
   }

   /**
    * Writes the configuration settings to a file.
    *
    * @param header a header to include at the top of the file.
    * @return true if successful, false if not.
    */
   public synchronized boolean write(String header)
   {
      boolean rval = false;
      
      try
      {
         // write the configuration to a file
         mProperties.store(new FileOutputStream(mFilename), header);
         rval = true;
      }
      catch(Throwable t)
      {
         getLogger().error(
               "Could not write config file: \"" + mFilename + "\"");
         getLogger().debug(Logger.getStackTrace(t));
      }

      return rval;
   }
   
   /**
    * Sets the header to write to the file, if any.
    * 
    * @param header the header to write to the file. 
    */
   public void setHeader(String header)
   {
      mHeader = header;
   }
   
   /**
    * Writes the configuration in the passed config options to this
    * config file.
    * 
    * @param config the configuration options to write to disk.
    * @return true if successfully written, false if not.
    */
   public boolean writeConfig(ConfigOptions config)
   {
      boolean rval = false;
      
      Iterator i = config.getKeys().iterator();
      while(i.hasNext())
      {
         String key = (String)i.next();
         setValue(key, config.getString(key));
      }
      
      rval = write();
      
      return rval;
   }
   
   /**
    * Reads from this configuration file and populates the passed
    * configuration options with the read options. 
    * 
    * @param config the configuration options to populate.
    * @param read true if the configuration should be read from file,
    *        false if it should not be.
    */
   public void populateConfig(ConfigOptions config, boolean read)
   {
      // read from file
      if(read)
      {
         read = read();
      }
      else
      {
         read = true;
      }
      
      if(read)
      {
         // copy all values read from file into the configuration
         Iterator i = getKeys().iterator();
         while(i.hasNext())
         {
            String key = (String)i.next();
            config.setValue(key, getString(key));
         }
      }
   }
}
