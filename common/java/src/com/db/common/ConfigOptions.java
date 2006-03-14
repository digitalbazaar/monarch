/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import com.db.common.logging.Logger;
import com.db.common.logging.LoggerManager;

import java.util.HashMap;
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
   protected HashMap mValueTypeMap;
   
   /**
    * A list of commands detected when last parsing a set of arguments.
    */
   protected Vector mCommands;
   
   /**
    * Creates a new config options object.
    */
   public ConfigOptions()
   {
      mProperties = new Properties();
      mValueTypeMap = new HashMap();
      mCommands = new Vector();
   }
   
   /**
    * Checks a value to see if it is the appropriate type.
    * 
    * @param key the key for the value.
    * @param value the value to check.
    * @return true if the value is the appropriate type, false if not.
    */
   protected boolean checkValueType(String key, Object value)
   {
      boolean rval = false;
      
      Class c = (Class)mValueTypeMap.get(key);
      if(c != null && value != null)
      {
         // ensure that the value is an instance of the restricted class
         if(!c.isInstance(value))
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
                  getLogger().debug(Logger.getStackTrace(t));
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
    * @return a list of arguments that could not be set.
    */
   public String[] parseArgs(String[] args)
   {
      Vector invalidArgs = new Vector();
      
      // a vector for storing commands
      mCommands = new Vector();
      
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
            getLogger().debug("could not parse argument!");
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
         ia[i] = (String)invalidArgs.get(i);
      }
      
      return ia;
   }
   
   /**
    * Converts a string to its appropriate type. Throws an exception if
    * the string could not be converted.
    * 
    * @param value the string to convert.
    * @param type the type to convert the string to.
    * @return the object the string was converted into.
    * @throws Exception
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
   public void setValueType(String key, Class type)
   {
      mValueTypeMap.put(key, type);
   }

   /**
    * Gets the type for a value.
    *
    * @param key the key for the value to get the type for.
    * @return the valid class for the value or null if no type
    *         restriction is set.
    */
   public Class getValueType(String key)
   {
      return (Class)mValueTypeMap.get(key);
   }
   
   /**
    * Gets a configuration value as a string.
    * 
    * @param key the key of the key-value pair.
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
         getLogger().error("could not read key,key=" + key);
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Gets a configuration value as an int.
    * 
    * @param key the key of the key-value pair.
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
         getLogger().error("could not read key,key=" + key);
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return rval;
   }   
   
   /**
    * Gets a configuration value as a double.
    * 
    * @param key the key of the key-value pair.
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
         getLogger().error("could not read key,key=" + key);
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return rval;
   }     
   
   /**
    * Gets a configuration value as a boolean.
    * 
    * @param key the key of the key-value pair.
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
         getLogger().error("could not read key,key=" + key);
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Sets a key-value pair.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
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
            mProperties.setProperty(key, null);
         }
      }
      
      return rval;
   }

   /**
    * Sets a key-value pair.
    * 
    * @param key the key of the key-value pair.
    * @param value the value of the key-value pair.
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
   public Set getKeys()
   {
      return mProperties.keySet();
   }
   
   /**
    * Gets the set of commands detected when the last set of arguments
    * was parsed. 
    * 
    * @return a vector command strings.
    */
   public Vector getCommands()
   {
      return mCommands;
   }
   
   /**
    * Clears this config options object of all values.
    */
   public void clear()
   {
      mProperties.clear();
      mValueTypeMap.clear();
      mCommands.clear();
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbcommon");
   }
}
