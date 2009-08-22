/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Collections;
using System.Runtime.CompilerServices;

using DB.Common.Logging;

namespace DB.Common
{
   /// <summary>
   /// A class for managing configuration options.
   /// </summary>
   ///
   /// <author>Dave Longley</author>
   public class ConfigOptions
   {
      /// <summary>
      /// The hashtable containing configuration options.
      /// </summary>
      protected Hashtable mOptions;
      
      /// <summary>
      /// A map of keys to their value's valid type.
      /// </summary>
      protected Hashtable mValueTypeMap;
      
      /// <summary>
      /// A list of commands detected when last parsing a set of arguments.
      /// </summary>
      protected ArrayList mCommands;
      
      /// <summary>
      /// Creates a new config options object.
      /// </summary>
      public ConfigOptions()
      {
         mOptions = new Hashtable();
         mValueTypeMap = new Hashtable();
         mCommands = new ArrayList();
      }
      
      /// <summary>
      /// Checks a value to see if it is the appropriate type.
      /// </summary>
      ///
      /// <param name="key">the key for the value.</param>
      /// <param name="val">the value to check.</param>
      /// <returns>true if the value is the appropriate type, false
      /// if not.</returns>
      protected virtual bool CheckValueType(string key, object val)
      {
         bool rval = false;
         
         Type t = (Type)mValueTypeMap[key];
         if(t != null && val != null)
         {
            // ensure that the value is an instance of the restricted class
            rval = t.IsInstanceOfType(val);
         }
         else
         {
            // no restrictions placed on value type
            rval = true;
         }
         
         return rval;
      }
      
      /// <summary>
      /// Gets a clone of this configuration.
      /// </summary>
      ///
      /// <returns>a clone of this configuration.</returns>
      public virtual object Clone()
      {
         ConfigOptions clone = new ConfigOptions();
         
         // FIXME: make sure this does a deep copy
         clone.mOptions = (Hashtable)mOptions.Clone();
         clone.mValueTypeMap = (Hashtable)mValueTypeMap.Clone();
         clone.mCommands = (ArrayList)mCommands.Clone();
         
         return clone;
      }
      
      /// <summary>
      /// Loads the configuration from an argument string.
      /// </summary>
      /// 
      /// <param name="args">the arguments string.</param>
      /// <returns>a list of arguments that could not be set.</returns>
      /// </summary>
      public virtual string[] ParseArgs(string[] args)
      {
         ArrayList invalidArgs = new ArrayList();
         
         // a list for storing commands
         mCommands = new ArrayList();
         
         string key = "";
         string val = "";
         bool setOption = false;
         
         string token = "";
         for(int i = 0; i < args.Length; i++)
         {
            token = args[i];
            
            try
            {
               if(key.Length > 0 && (setOption || token.StartsWith("-")))
               {
                  // try to set the value
                  if(!SetValue(key, val))
                  {
                     // value could not be set, argument is invalid
                     invalidArgs.Add(key);
                  }
                  
                  key = "";
                  val = "";
                  setOption = false;
               }
               
               if(token.StartsWith("-") && token.Length > 1)
               {
                  if(token.StartsWith("--") && token.Length > 2)
                  {
                     // double-hyphen option, strip off the hyphens
                     token = token.Substring(2);
                     
                     // check for an equals sign
                     int index = token.IndexOf('=');
                     if(index != -1)
                     {
                        // parse around the equals sign
                        key = token.Substring(0, index);
                        if((index + 1) < token.Length)
                        {
                           val = token.Substring(index + 1);
                        }
                     }
                     else
                     {
                        // if no equals sign, the key is the rest of the token
                        key = token;
                     }
                  }
                  else if(token.StartsWith("-"))
                  {
                     // single hyphen option, strip off the hyphen
                     token = token.Substring(1);
                     
                     // get the key
                     key = token.Substring(0, 1);
                     if(token.Length > 1)
                     {
                        // get the value
                        val = token.Substring(1, token.Length - 1);
                        setOption = true;
                     }
                  }
               }
               else if(key == "")
               {
                  // this is a command, so append it to the command vector
                  mCommands.Add(token);
               }
               else
               {
                  // this is part of a value for a key-value pair, so append it
                  val = (val.Length > 0) ? (val + " " + token) : token;
               }
            }
            catch(Exception e)
            {
               Logger.Error("could not parse argument!");
               Logger.Debug(e.ToString());
               key = "";
            }
         }
         
         // set any remaining value
         if(key.Length > 0)
         {
            // try to set the value
            if(!SetValue(key, val))
            {
               // value could not be set, argument is invalid
               invalidArgs.Add(key);
            }
         }
         
         string[] ia = new string[invalidArgs.Count];
         for(int i = 0; i < invalidArgs.Count; i++)
         {
            ia[i] = (string)invalidArgs[i];
         }
         
         return ia;
      }
      
      /// <summary>
      /// Sets the type for a value.
      /// </summary>
      /// 
      /// <param name="key">the key for the value to set the type for.</param>
      /// <param name="type">the type for the value.</param>
      public virtual void SetValueType(string key, Type type)
      {
         mValueTypeMap[key] = type;
      }
      
      /// <summary>
      /// Gets the type for a value.
      /// </summary>
      /// 
      /// <param name="key">the key for the value to get the type for.</param>
      /// <returns>the type for the value or null if no type restriction
      /// is set.</returns>
      public virtual Type GetValueType(string key)
      {
         return (Type)mValueTypeMap[key];
      }
      
      /// <summary>
      /// Gets a configuration value as a string.
      /// </summary>
      /// 
      /// <param name="key">the key of the key-value pair.</param>
      /// <returns>the configuration value as an object, null if the key is
      /// not found.</returns>
      public virtual object GetValue(string key)
      {
         return mOptions[key];
      }
      
      /// <summary>
      /// Gets a configuration value as a string.
      /// </summary>
      /// 
      /// <param name="key">the key of the key-value pair.</param>
      /// <returns>the configuration value as a string, a blank string if the
      /// key is not found.</returns>
      public virtual string GetString(string key)
      {
         string rval = "";
         
         object obj = mOptions[key];
         if(obj != null)
         {
            rval = obj.ToString();
         }
         
         return rval;
      }
      
      /// <summary>
      /// Gets a configuration value as a long.
      /// </summary>
      /// 
      /// <param name="key">the key of the key-value pair.</param>
      /// <returns>the configuration value as a long.</returns>
      public virtual long GetLong(string key)
      {
         long rval = 0;
         
         try
         {
            rval = Convert.ToInt64(GetValue(key));
         }
         catch(Exception e)
         {
            Logger.Error("could not read key,key=" + key);
            Logger.Debug(e.ToString());
         }
         
         return rval;
      }
      
      /// <summary>
      /// Gets a configuration value as an int.
      /// </summary>
      /// 
      /// <param name="key">the key of the key-value pair.</param>
      /// <returns>the configuration value as an int.</returns>
      public virtual int GetInt(string key)
      {
         int rval = 0;
         
         try
         {
            rval = Convert.ToInt32(GetValue(key));
         }
         catch(Exception e)
         {
            Logger.Error("could not read key,key=" + key);
            Logger.Debug(e.ToString());
         }
         
         return rval;
      }

      /// <summary>
      /// Gets a configuration value as a double.
      /// </summary>
      /// 
      /// <param name="key">the key of the key-value pair.</param>
      /// <returns>the configuration value as a double.</returns>
      public virtual double GetDouble(string key)
      {
         double rval = 0;
         
         try
         {
            rval = Convert.ToDouble(GetValue(key));
         }
         catch(Exception e)
         {
            Logger.Error("could not read key,key=" + key);
            Logger.Debug(e.ToString());
         }
         
         return rval;
      }
      
      /// <summary>
      /// Gets a configuration value as a boolean.
      /// </summary>
      /// 
      /// <param name="key">the key of the key-value pair.</param>
      /// <returns>the configuration value as a boolean.</returns>
      public virtual bool GetBoolean(string key)
      {
         bool rval = false;
         
         try
         {
            object obj = GetValue(key);
            if(obj is String && (string)obj == "1")
            {
               rval = true;
            }
            else
            {
               rval = Convert.ToBoolean(obj);
            }
         }
         catch(Exception e)
         {
            Logger.Error("could not read key,key=" + key);
            Logger.Debug(e.ToString());
         }
         
         return rval;
      }
      
      /// <summary>
      /// Sets a key-value pair.
      /// </summary>
      /// 
      /// <param name="key">the key of the key-value pair.</param>
      /// <param name="val">the value of the key-value pair.</param>
      /// <returns>true if the value could be set, false if not.</returns>
      [MethodImpl(MethodImplOptions.Synchronized)]
      public virtual bool SetValue(string key, object val)
      {
         bool rval = false;
         
         if(CheckValueType(key, val))
         {
            rval = true;
            mOptions[key] = val;
         }
         
         return rval;
      }
      
      /// <summary>
      /// Gets/Sets a key-value pair. An ArgumentException will be thrown
      /// if the passed value is not the appropriate type (as set previously
      /// by a call to SetValueType()).
      /// </summary>
      /// 
      /// <param name="key">the key for the key-value pair.</param>
      /// <returns>the value for the key-value pair.</returns>
      public virtual object this[string key]
      {
         get
         {
            return GetValue(key);
         }
         
         set
         {
            if(!SetValue(key, value))
            {
               throw new ArgumentException(
               "Value is not the appropriate type! Type for values with " +
               "key \"" + key + "\" must be \"" + GetValueType(key) + "\"");
            }
         }
      }
      
      /// <summary>
      /// Gets a set of all the keys.
      /// </summary>
      /// 
      /// <returns>a set of all the keys.</returns>
      public virtual ICollection Keys
      {
         get
         {
            return mOptions.Keys;
         }
      }
      
      /// <summary>
      /// Gets the set of commands detected when the last set of arguments
      /// was parsed.
      /// </summary>
      /// 
      /// <returns>a list of command strings.</returns>
      public virtual ArrayList Commands
      {
         get
         {
            return mCommands;
         }
      }
      
      /// <summary>
      /// Gets the logger.
      /// </summary>
      ///
      /// <returns>the logger.</returns>
      public virtual Logger Logger
      {
         get
         {
            return LoggerManager.GetLogger("dbcommon");
         }
      }
   }
}
