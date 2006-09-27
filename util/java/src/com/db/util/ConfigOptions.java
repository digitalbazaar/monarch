/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

import java.io.File;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Properties;
import java.util.Set;
import java.util.Vector;

/**
 * A class for managing configuration options.
 * 
 * @author Dave Longley
 */
public class ConfigOptions
{
   /**
    * The properties object containing configuration settings.
    */
   protected Properties mProperties;
   
   /**
    * A map of keys to their value's valid type.
    */
   protected HashMap<String, Class> mValueTypeMap;
   
   /**
    * A list of commands detected when last parsing a set of arguments.
    */
   protected Vector<String> mCommands;
   
   /**
    * Creates a new config options object.
    */
   public ConfigOptions()
   {
      mProperties = new Properties();
      mValueTypeMap = new HashMap<String, Class>();
      mCommands = new Vector<String>();
   }
   
   /**
    * Checks a value to see if it is the appropriate type.
    * 
    * @param key the key for the value.
    * @param value the value to check.
    * 
    * @return true if the value is the appropriate type, false if not.
    */
   protected boolean checkValueType(String key, Object value)
   {
      boolean rval = false;
      
      Class c = mValueTypeMap.get(key);
      if(c != null && value != null)
      {
         // ensure that the value is an instance of the restricted class
         if(c.isInstance(value))
         {
            // value is instance of class
            rval = true;
         }
         else
         {
            // if type is a primitive and value is a string, we may be
            // able to parse the string to the correct type
            if(c.isPrimitive() && value instanceof String)
            {
               try
               {
                  // try to parse out the appropriate object
                  parseObject((String)value, c);
                  
                  // no exception thrown, so return true
                  rval = true;
               }
               catch(Throwable t)
               {
                  getLogger().debug(getClass(), Logger.getStackTrace(t));
               }
            }
         }
      }
      else
      {
         // no restrictions placed on value type
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Loads the configuration from an argument string.
    * 
    * @param args the arguments string.
    * 
    * @return a list of arguments that could not be set.
    */
   public String[] parseArgs(String[] args)
   {
      Vector<String> invalidArgs = new Vector<String>();
      
      // a vector for storing commands
      mCommands = new Vector<String>();
      
      String key = "";
      String value = "";
      boolean setOption = false;
      
      String token = "";
      int length = args.length;
      for(int i = 0; i < length; i++)
      {
         token = args[i];
         
         try
         {
            if(key.length() > 0 && (setOption || token.startsWith("-")))
            {
               // try to set the value
               if(!setValue(key, value))
               {
                  // value could not be set, argument is invalid
                  invalidArgs.add(key);
               }
               
               key = "";
               value = "";
               setOption = false;
            }
            
            if(token.startsWith("-") && token.length() > 1)
            {
               if(token.startsWith("--") && token.length() > 2)
               {
                  // double-hyphen option, strip off the hyphens
                  token = token.substring(2);
                  
                  // check for an equals sign
                  int index = token.indexOf('=');
                  if(index != -1)
                  {
                     // parse around the equals sign
                     key = token.substring(0, index);
                     if((index + 1) < token.length())
                     {
                        value = token.substring(index + 1);
                     }
                  }
                  else
                  {
                     // if no equals sign, the key is the rest of the token
                     key = token;
                  }
               }
               else if(token.startsWith("-"))
               {
                  // single hyphen option, strip off the hyphen
                  token = token.substring(1);
                     
                  // get the key
                  key = token.substring(0, 1);
                  if(token.length() > 1)
                  {
                     // get the value
                     value = token.substring(1, token.length());
                     setOption = true;
                  }
               }
            }
            else if(key.equals(""))
            {
               // this is a command, so append it to the command vector
               mCommands.add(token);
            }
            else
            {
               // this is part of a value for a key-value pair, so append it
               value = (value.length() > 0) ? (value + " " + token) : token; 
            }
         }
         catch(Throwable t)
         {
            getLogger().debug(getClass(), "could not parse argument!");
            key = "";
         }
      }

      // set any remaining value
      if(key.length() > 0)
      {
         // try to set the value
         if(!setValue(key, value))
         {
            // value could not be set, argument is invalid
            invalidArgs.add(key);
         }
      }
      
      String[] ia = new String[invalidArgs.size()];
      for(int i = 0; i < invalidArgs.size(); i++)
      {
         ia[i] = invalidArgs.get(i);
      }
      
      return ia;
   }
   
   /**
    * Converts a string to its appropriate type. Throws an exception if
    * the string could not be converted.
    * 
    * @param value the string to convert.
    * @param type the type to convert the string to.
    * 
    * @return the object the string was converted into.
    */
   public static Object parseObject(String value, Class type)
   {
      Object rval = null;
      
      if(value != null)
      {
         if(type == String.class)
         {
            rval = value;
         }
         else if(type == boolean.class || type == Boolean.class)
         {
            boolean b = Boolean.parseBoolean(value);
            rval = new Boolean(b);
         }
         else if(type == byte.class || type == Byte.class)
         {
            byte b = value.getBytes()[0];
            rval = new Byte(b);
         }
         else if(type == char.class || type == Character.class)
         {
            char c = value.charAt(0);
            rval = new Character(c);
         }
         else if(type == short.class || type == Short.class)
         {
            short s = Short.parseShort(value);
            rval = new Short(s);
         }
         else if(type == int.class || type == Integer.class)
         {
            int i = Integer.parseInt(value);
            rval = new Integer(i);
         }
         else if(type == long.class || type == Long.class)
         {
            long l = Long.parseLong(value);
            rval = new Long(l);
         }
         else if(type == float.class || type == Float.class)
         {
            float f = Float.parseFloat(value);
            rval = new Float(f);
         }
         else if(type == double.class || type == Double.class)
         {
            double d = Double.parseDouble(value);
            rval = new Double(d);
         }
      }

      return rval;
   }   
   
   /**
    * Sets the type for a value.
    * 
    * @param key the key for the value to set the type for.
    * @param type the class for the value.
    */
   public synchronized void setValueType(String key, Class type)
   {
      mValueTypeMap.put(key, type);
   }

   /**
    * Gets the type for a value.
    *
    * @param key the key for the value to get the type for.
    * 
    * @return the valid class for the value or null if no type
    *         restriction is set.
    */
   public Class getValueType(String key)
   {
      return mValueTypeMap.get(key);
   }
   
   /**
    * Returns whether or not a configuration key exists.
    * 
    * Same as hasKey().
    * 
    * @param key the key of the key-value pair.
    * 
    * @return true if the key exists, false if not.
    */
   public boolean keyExists(String key)
   {
      return (mProperties.getProperty(key) != null);
   }
   
   /**
    * Returns whether or not a configuration key exists.
    * 
    * Same as keyExists().
    * 
    * @param key the key of the key-value pair.
    * 
    * @return true if the key exists, false if not.
    */
   public boolean hasKey(String key)
   {
      return keyExists(key);
   }
   
   /**
    * Gets a configuration value as a string.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the configuration value as a string, null if the key is not found.
    */
   public String getValue(String key)
   {
      return mProperties.getProperty(key);
   }   
   
   /**
    * Gets a configuration value as a string.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the configuration value as a string, a blank string if the
    *         key is not found.
    */
   public String getString(String key)
   {
      String rval = "";
      
      String str = mProperties.getProperty(key);
      if(str != null)
      {
         rval = str;
      }
      
      return rval;
   }
   
   /**
    * Gets a configuration value as a long.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the configuration value as a long.
    */
   public long getLong(String key)
   {
      long rval = 0;
      
      String value = getString(key);
      
      try
      {
         rval = Long.parseLong(value);
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), "could not read key,key=" + key);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Gets a configuration value as an int.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the configuration value as an int.
    */
   public int getInt(String key)
   {
      int rval = 0;
      
      String value = getString(key);
      try
      {
         rval = Integer.parseInt(value);
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), "could not read key,key=" + key);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }   
   
   /**
    * Gets a configuration value as a double.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the configuration value as a double.
    */
   public double getDouble(String key)
   {
      double rval = 0;
      
      String value = getString(key);
      try
      {
         rval = Double.parseDouble(value);
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), "could not read key,key=" + key);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }     
   
   /**
    * Gets a configuration value as a boolean.
    * 
    * @param key the key of the key-value pair.
    * 
    * @return the configuration value as a boolean.
    */
   public boolean getBoolean(String key)
   {
      boolean rval = false;
      
      String value = getString(key);
      try
      {
         if(value.equals("0"))
         {
            rval = false;
         }
         else if(value.equals("1"))
         {
            rval = true;
         }
         else
         {
            rval = Boolean.parseBoolean(value);
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), "could not read key,key=" + key);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Sets a key-value pair.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    * 
    * @return true if the value could be set, false if not.
    */
   public synchronized boolean setValue(String key, Object value)
   {
      boolean rval = false;
      
      if(checkValueType(key, value))
      {
         rval = true;
         
         if(value != null)
         {
            mProperties.setProperty(key, value.toString());
         }
         else
         {
            mProperties.remove(key);
         }
      }
      
      return rval;
   }

   /**
    * Sets a key-value pair.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    * 
    * @return true if the value could be set, false if not.
    */
   public boolean setValue(String key, String value)
   {
      return setValue(key, (Object)value);
   }
   
   /**
    * Sets a key-value pair.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    * 
    * @return true if the value could be set, false if not.
    */
   public boolean setValue(String key, long value)
   {
      return setValue(key, new Long(value));
   }
   
   /**
    * Sets a key-value pair.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    * 
    * @return true if the value could be set, false if not.
    */
   public boolean setValue(String key, int value)
   {
      return setValue(key, new Integer(value));
   }
   
   /**
    * Sets a key-value pair.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    * 
    * @return true if the value could be set, false if not.
    */
   public boolean setValue(String key, double value)
   {
      return setValue(key, new Double(value));
   }
   
   /**
    * Sets a key-value pair.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
    * 
    * @return true if the value could be set, false if not.
    */
   public boolean setValue(String key, boolean value)
   {
      return setValue(key, new Boolean(value));
   }
   
   /**
    * Gets a set of all the keys.
    * 
    * @return a set of all the keys.
    */
   public Set<Object> getKeys()
   {
      return mProperties.keySet();
   }
   
   /**
    * Gets a set of all the keys that begin with the passed string.
    * 
    * @param prefix the prefix for the keys.
    * 
    * @return a set of all the keys that begin with the passed string.
    */
   public synchronized Set<String> getKeysStartingWith(String prefix)
   {
      HashSet<String> set = new HashSet<String>();
      
      for(Object key: getKeys())
      {
         String strKey = (String)key;
         if(strKey.startsWith(prefix))
         {
            set.add(strKey);
         }
      }
      
      return set;
   }
   
   /**
    * Gets the set of commands detected when the last set of arguments
    * was parsed. 
    * 
    * @return a vector command strings.
    */
   public Vector<String> getCommands()
   {
      return mCommands;
   }
   
   /**
    * Clears this config options object of all values.
    */
   public synchronized void clear()
   {
      mProperties.clear();
      mValueTypeMap.clear();
      mCommands.clear();
   }
   
   /**
    * Reads the configuration options from the passed configuration
    * options object into this one. Every value in the passed configuration
    * that has the same value type as this configuration will be used.
    * 
    * @param config the config options object to read from.
    */
   public synchronized void readConfigFrom(ConfigOptions config)
   {
      readConfigFrom(config, true);
   }
   
   /**
    * Reads the configuration options from the passed configuration
    * options object into this one. If overwrite is set, then every value
    * in the passed configuration that has the same value type as this
    * configuration will be used. If overwrite is not set, only those
    * values that are new in the passed configuration will be added to
    * this one.
    * 
    * @param config the config options object to read from.
    * @param overwrite true to overwrite existing options, false to only
    *                  copy new ones.
    */
   public synchronized void readConfigFrom(
      ConfigOptions config, boolean overwrite)
   {
      if(config != null)
      {
         if(overwrite)
         {
            // copy properties
            for(Object key: config.mProperties.keySet())
            {
               String strKey = (String)key;
               setValue(strKey, config.getValue(strKey));
            }
         }
         else
         {
            // copy properties
            for(Object key: config.mProperties.keySet())
            {
               String strKey = (String)key;
               
               // only update value if this config doesn't have it
               if(!hasKey(strKey))
               {
                  setValue(strKey, config.getValue(strKey));
               }
            }
         }
      }
   }   
   
   /**
    * Writes the configuration options in this object to the passed
    * configuration options object. Every value in this configuration
    * that has the same value type as the passed configuration will be used.
    * 
    * @param config the config options object to write to.
    */
   public synchronized void writeConfigTo(ConfigOptions config)
   {
      if(config != null)
      {
         // copy properties
         for(Object key: mProperties.keySet())
         {
            String strKey = (String)key;
            config.setValue(strKey, getValue(strKey));
         }
      }
   }
   
   /**
    * Copies this config options object to another one.
    * 
    * @return the config options copy.
    */
   public synchronized ConfigOptions copy()
   {
      ConfigOptions config = new ConfigOptions();
      
      // copy commands
      config.mCommands.addAll(mCommands);
      
      // copy value hash map
      for(String key: mValueTypeMap.keySet())
      {
         config.setValueType(key, getValueType(key));
      }

      // write to config
      writeConfigTo(config);
      
      return config;
   }
   
   /**
    * Convenience method. Tries to create the passed directory if it
    * does not exist.
    * 
    * @param dir the directory.
    * 
    * @return true the directory exists or was created, false if not.
    */
   public boolean createDir(String dir)
   {
      boolean rval = false;
      
      File f = new File(dir);
      if(!f.exists())
      {
         if(f.mkdirs())
         {
            rval = true;
         }
      }
      else
      {
         rval = true;
      }
      
      return rval;      
   }   
   
   /**
    * Sets a directory by first creating the directory if it doesn't exist.
    * 
    * @param key the key to the directory.
    * @param dir the directory to create and store as a configuration value.
    * 
    * @return true if set properly and directory exists, false if not.
    */
   public boolean setDir(String key, String dir)
   {
      boolean rval = false;
      
      if(!dir.endsWith(File.separator))
      {
         dir += File.separator;
      }
      
      if(createDir(dir))
      {
         setValue(key, dir); 
      }
      
      return rval;
   }
   
   /**
    * Gets a directory by first creating the directory if it doesn't exist.
    * 
    * @param key the key to the directory.
    * 
    * @return the directory.
    */
   public String getDir(String key)
   {
      String dir = getString(key);
      setDir(key, dir);
      dir = getString(key);
      
      return dir;
   }
   
   /**
    * Convenience method. Compares two configuration values between
    * two different configurations.
    * 
    * @param config1 the first configuration.
    * @param config2 the second configuration.
    * @param key the configuration key.
    * 
    * @return true if the options are the same, false if not.
    */
   public static boolean compareConfigValue(
      ConfigOptions config1, ConfigOptions config2, String key)
   {
      boolean rval = false;
      
      if(config1 != null && config2 != null)
      {
         Object value1 = config1.getValue(key);
         Object value2 = config2.getValue(key);
         if((value1 == null && value2 == null) ||
            (value1 != null && value1.equals(value2)))
         {
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbutil");
   }
}
