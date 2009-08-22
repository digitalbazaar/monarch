/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Collections;
using System.Globalization;
using System.IO;
using System.Runtime.CompilerServices;

using DB.Common.Logging;

namespace DB.Common
{
   /// <summary>
   /// A class for managing a configuration file.
   /// </summary>
   ///
   /// <author>Dave Longley</author>
   public class ConfigFile : ConfigOptions
   {
      /// <summary>
      /// The name of the config file.
      /// </summary>
      protected string mFilename;
      
      /// <summary>
      /// A header to write in the file.
      /// </summary>
      protected string mHeader;
      
      /// <summary>
      /// Creates a new config file.
      /// </summary>
      /// 
      /// <param name="filename">the name of the config file.</param>
      public ConfigFile(string filename)
      {
         mFilename = filename;
         mHeader = null;
      }
      
      /// <summary>
      /// Loads the configuration settings from a file.
      /// </summary>
      ///
      /// <returns>true if successful, false if not.</returns>
      [MethodImpl(MethodImplOptions.Synchronized)]
      public virtual bool Read()
      {
         bool rval = false;
         
         mOptions = new Hashtable();
         
         try
         {
            // read the configuration from a file
            TextReader tr = File.OpenText(mFilename);
            
            string line = null;
            while((line = tr.ReadLine()) != null)
            {
               // only parse the line if it starts with a comment character
               if(!line.StartsWith("#"))
               {
                  // read to the first equals
                  int equals = line.IndexOf("=");
                  if(equals != -1)
                  {
                     string key = line.Substring(0, equals);
                     string val = "";
                     if((equals + 1) < line.Length)
                     {
                        // set the value to the rest of the line
                        val = line.Substring(equals + 1,
                                             line.Length - (equals + 1));
                     }
                     
                     mOptions[key] = val;
                  }
                  else
                  {
                     throw new System.FormatException(
                     "Configuration option is not of the format \"key=value\"");
                  }
               }
            }
            
            rval = true;
         }
         catch(Exception e)
         {
            Logger.Error("Could not read config file: \"" + mFilename + "\"");
            Logger.Debug(e.ToString());
         }
         
         return rval;
      }
      
      /// <summary>
      /// Writes the configuration settings to a file.
      /// </summary>
      /// 
      /// <returns>true if successful, false if not.</returns>
      public virtual bool Write()
      {
         return Write(mHeader);
      }
      
      /// <summary>
      /// Writes the configuration settings to a file.
      /// </summary>
      /// 
      /// <param name="header">header a header to include at the top of
      /// the file.</param>
      /// <returns>true if successful, false if not.</returns>
      [MethodImpl(MethodImplOptions.Synchronized)]
      public virtual bool Write(string header)
      {
         bool rval = false;
         
         try
         {
            // delete existing config file
            File.Delete(mFilename);

            // write the configuration to a file
            TextWriter tw = File.CreateText(mFilename);
            
            // write header to file if one exists
            if(Header != null)
            {
               tw.WriteLine("# " + mHeader);
            }
            
            // write date to file
            string df = "M/d/yyyy h:mm:ss tt 'GMT' z";
            DateTime now = DateTime.Now;
            string date = now.ToString(df, DateTimeFormatInfo.InvariantInfo);
            tw.WriteLine("# " + date);
            
            // write the key-value pairs to file
            foreach(string key in Keys)
            {
               tw.WriteLine(key + "=" + GetString(key));
            }
            
            rval = true;
         }
         catch(Exception e)
         {
            Logger.Error("Could not write config file: \"" + mFilename + "\"");
            Logger.Debug(e.ToString());
         }
         
         return rval;
      }
      
      /// <summary>
      /// Gets/Sets the header to write to the file, if any.
      /// </summary>
      public virtual string Header
      {
         get
         {
            return mHeader;
         }
         
         set
         {
            mHeader = value;
         }
      }
   }
}
