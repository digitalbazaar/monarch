/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Collections;
using System.IO;

namespace DB.Common.Logging
{
   /// <summary>
   /// A class to manage loggers.
   /// </summary>
   ///
   /// <author>Dave Longley</author>
   public class LoggerManager
   {
      /// <summary>
      /// The singleton instance of a logger manager.
      /// </summary>
      protected static LoggerManager smInstance = new LoggerManager();

      /// <summary>
      /// A map of logger names to loggers.
      /// </summary>
      protected static Hashtable smLoggers;

      /// <summary>
      /// Creates the logger manager.
      /// </summary>
      protected LoggerManager()
      {
         smLoggers = new Hashtable();
      }
      
      /// <summary>
      /// Gets the singleton instance of the logger manager.
      /// </summary>
      ///
      /// <returns>the singleton instance of the logger manager.</returns>
      public static LoggerManager Instance
      {
         get
         {
            return smInstance;
         }
      }
      
      /// <summary>
      /// Creates a new logger with the passed name. If the name already
      /// exists, the logger will not be created and this method
      /// will return false.
      /// </summary>
      ///
      /// <param name="name">the name for the new logger.</param>
      /// <returns>true if successfully created, false if not.</returns>
      public static bool CreateLogger(string name)
      {
         bool rval = false;

         if(smLoggers[name] == null)
         {
            // create a new logger
            Logger logger = new Logger(name);
            
            // add the logger
            smLoggers[name] = logger;
            
            rval = true;
         }

         return rval;
      }

      /// <summary>
      /// Gets the logger with the passed name.
      /// Creates a new logger if necessary.
      /// </summary>
      ///
      /// <param name="name">the name of the logger to get.</param>
      /// <returns>the logger</returns>
      public static Logger GetLogger(string name)
      {
         Logger logger = (Logger)smLoggers[name];
         if(logger == null)
         {
            if(CreateLogger(name))
            {
               logger = GetLogger(name);
            }
         }
            
         return logger;
      }

      /// <summary>
      /// Sets the verbosity for the specified Logger. If the verbosity
      /// passed is not in the accepted range, this method will
      /// fail to set the verbosity and return false. Otherwise
      /// it will set the verbosity and return true.
      /// </summary>
      ///
      /// <param name="name">the name of the logger.</param>
      /// <param name="fileVerbosity">the file verbosity to set.</param>
      /// <param name="consoleVerbosity">the console verbosity to set.</param>
      /// <returns>true if verbosity valid and set, false if not.</returns>
      public static bool SetVerbosity(string name, int fileVerbosity,
                                      int consoleVerbosity)
      {
         bool rval = false;

         Logger logger = GetLogger(name);
         if(logger != null)
         {
            logger.FileVerbosity = fileVerbosity;         
            logger.ConsoleVerbosity = consoleVerbosity;
            
            rval = (logger.FileVerbosity == fileVerbosity);
            rval &= (logger.ConsoleVerbosity == consoleVerbosity); 
         }

         return rval;
      }

      /// <summary>
      /// Uses the specified Logger. Opens a new log file with the
      /// specified file name. Will append if file already exists.
      /// </summary>
      ///
      /// <param name="name">name the name of the logger.</param>
      /// <param name="filename">the name of the log file.</param>
      /// <returns>true if succesfully opened the file for writing, false
      /// if not.</returns>
      public static bool SetFile(string name, string filename)
      {
         return SetFile(name, filename, true);
      }

      /// <summary>
      /// Uses the specified logger. Opens a new log file with the specified
      /// file name. Setting append to true will append to the
      /// file if it exists. Setting it to false will overwrite it.
      /// </summary>
      ///
      /// <param name="name">the name of the logger.</param>
      /// <param name="filename">the name of the log file.</param>
      /// <param name="append">specifies whether or not to append to an existing
      /// file or to overwrite.</param>
      /// <returns>true if succesfully opened the file for writing,
      /// false if not.</returns>
      public static bool SetFile(string name, string filename, bool append)
      {
         bool rval = false;
         
         Logger logger = GetLogger(name);
         if(logger != null)
         {
            rval = logger.SetFile(filename, append);
         }
         else
         {
            // create the logger
            if(CreateLogger(name))
            {
               rval = SetFile(name, filename, append);
            }
         }

         return rval;
      }
      
      /// <summary>
      /// Resets logger files for all loggers that were using the
      /// passed file (identified by file name).
      /// </summary>
      ///
      /// <param name="filename">the name of the file.</param>
      public static void ResetLoggerFiles(string filename)
      {
         foreach(Logger logger in smLoggers.Values)
         {
            if(logger.Filename.Equals(filename))
            {
               logger.SetFile(filename, true);
            }
         }
      }
      
      /// <summary>
      /// Adds a text writer to a logger.
      /// </summary>
      ///
      /// <param name="name">the name of the logger to modify.</param>
      /// <param name="tw">the text writer to add.</param>
      /// <param name="verbosity">the verbosity for the text writer.</param>
      /// <returns>true if successfully added, false if not.</returns>
      public static bool AddTextWriter(string name,
                                       TextWriter tw, int verbosity)
      {
         bool rval = false;
         
         Logger logger = GetLogger(name);
         if(logger != null)
         {
            logger.AddTextWriter(tw, verbosity);
            rval = true;
         }
         
         return rval;
      }
      
      /// <summary>
      /// Removes a logger's text writer.
      /// </summary>
      ///
      /// <param name="name">the name of the logger to modify.</param>
      /// <param name="tw">the text writer to remove.</param>
      /// <returns>true if successfully added, false if not.</returns>
      public static bool RemoveTextWriter(string name, TextWriter tw)
      {
         bool rval = false;
         
         Logger logger = GetLogger(name);
         if(logger != null)
         {
            logger.RemoveTextWriter(tw);
            rval = true;
         }
         
         return rval;
      }
      
      /// <summary>
      /// Sets a logger's text writer's verbosity.
      /// </summary>
      ///
      /// <param name="name">the name of the logger to modify.</param>
      /// <param name="tw">the text writer to modify.</param>
      /// <param name="verbosity">the verbosity for the text writer.</param>
      /// <returns>true if successfully added, false if not.</returns>
      public static bool SetTextWriterVerbosity(string name,
                                                TextWriter tw, int verbosity)
      {
         bool rval = false;

         Logger logger = GetLogger(name);
         if(logger != null)
         {
            logger.SetTextWriterVerbosity(tw, verbosity);
            rval = true;
         }
         
         return rval;
      }
      
      /// <summary>
      /// Uses the specified logger, if it exists. Writes the passed string to
      /// the log file, if it is open.
      /// </summary>
      ///
      /// <param name="name">the name of the logger to use.</param>
      /// <param name="text">the text to write to the log file.</param>
      /// <returns>true if the text was written, false if not.</returns>
      public static bool Log(string name, string text)
      {
         return Log(name, text, Logger.ERROR_VERBOSITY);
      }

      /// <summary>
      /// Uses the specified logger, if it exists. Writes the passed string to
      /// the log file.
      /// </summary>
      ///
      /// <param name="name">the name of the logger to use.</param>
      /// <param name="text">the text to write to the logger.</param>
      /// <param name="verbosity">the verbosity level that must be reached in
      /// order for the text to be written to the log.</param>
      /// <returns>true if the text was written, false if not.</returns>
      public static bool Log(string name, string text, int verbosity)
      {
         bool rval = false;

         Logger logger = GetLogger(name);
         if(logger != null)
         {
            rval = logger.Log(text, verbosity);
         }

         return rval;
      }

      /// <summary>
      /// Uses the specified Logger. Writes the passed string to
      /// the log file, if it is open. Verbosity is set to error level.
      /// </summary>
      ///
      /// <param name="name">the name of the logger to use.</param>
      /// <param name="text">the text to write to the logger.</param>
      /// <returns>true if the text was written, false if not.</returns>
      public static bool Error(string name, string text)
      {
         bool rval = false;

         Logger logger = GetLogger(name);
         if(logger != null)
         {
            rval = logger.Error(text);
         }

         return rval;
      }

      /// <summary>
      /// Uses the specified Logger. Writes the passed string to
      /// the log file, if it is open. Verbosity is set to warning level.
      /// </summary>
      ///
      /// <param name="name">the name of the logger to use.</param>
      /// <param name="text">the text to write to the logger.</param>
      /// <returns>true if the text was written, false if not.</returns>
      public static bool Warning(string name, string text)
      {
         bool rval = false;

         Logger logger = GetLogger(name);
         if(logger != null)
         {
            rval = logger.Warning(text);
         }

         return rval;
      }

      /// <summary>
      /// Uses the specified Logger. Writes the passed string to
      /// the log file, if it is open. Verbosity is set to debug level.
      /// </summary>
      ///
      /// <param name="name">the name of the logger to use.</param>
      /// <param name="text">the text to write to the logger.</param>
      /// <returns>true if the text was written, false if not.</returns>
      public static bool Debug(string name, string text)
      {
         bool rval = false;

         Logger logger = GetLogger(name);
         if(logger != null)
         {
            rval = logger.Debug(text);
         }

         return rval;
      }


      /// <summary>
      /// Uses the specified Logger. Writes the passed string to
      /// the log file, if it is open. Verbosity is set to data debug level.
      /// </summary>
      ///
      /// <param name="name">the name of the logger to use.</param>
      /// <param name="text">the text to write to the logger.</param>
      /// <returns>true if the text was written, false if not.</returns>
      public static bool DebugData(string name, string text)
      {
         bool rval = false;

         Logger logger = GetLogger(name);
         if(logger != null)
         {
            rval = logger.DebugData(text);
         }

         return rval;
      }

      /// <summary>
      /// Uses the specified Logger. Writes the passed string to
      /// the log file, if it is open. Verbosity is set to message level.
      /// </summary>
      ///
      /// <param name="name">the name of the logger to use.</param>
      /// <param name="text">the text to write to the logger.</param>
      /// <returns>true if the text was written, false if not.</returns>
      public static bool Msg(string name, string text)
      {
         bool rval = false;

         Logger logger = GetLogger(name);
         if(logger != null)
         {
            rval = logger.Msg(text);
         }

         return rval;
      }
   }
}
