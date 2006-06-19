/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.logging;

import java.io.PrintStream;
import java.util.HashMap;
import java.util.Iterator;

/**
 * A class to manage loggers.
 *  
 * @author Dave Longley
 */
public class LoggerManager
{
   /**
    * The singleton instance of a logger manager.
    */
   protected static LoggerManager smInstance = new LoggerManager();

   /**
    * A map of logger names to loggers.
    */
   protected static HashMap smLoggers;

   /**
    * Creates the logger manager.
    */
   protected LoggerManager()
   {
      smLoggers = new HashMap();
   }
   
   /**
    * Gets the singleton instance of the logger manager.
    * 
    * @return the singleton instance of the logger manager.
    */
   public static LoggerManager getInstance()
   {
      return smInstance;
   }
   
   /**
    * Creates a new logger with the passed name. If the name already
    * exists, the logger will not be created and this method
    * will return false.
    * 
    * @param name the name for the new logger.
    * @return true if successfully created, false if not.
    */
   public static boolean createLogger(String name)
   {
      boolean rval = false;

      if(smLoggers.get(name) == null)
      {
         // create a new logger
         Logger logger = new Logger(name);
         
         // add the logger
         smLoggers.put(name, logger);
         
         rval = true;
      }

      return rval;
   }

   /**
    * Gets the logger with the passed name. Creates a new logger if necessary.
    * 
    * @param name the name of the logger to get.
    * @return a pointer to the logger.
    */
   public static Logger getLogger(String name)
   {
      Logger logger = (Logger)smLoggers.get(name);
      if(logger == null)
      {
         createLogger(name);
         logger = getLogger(name);
      }
      
      return logger;
   }

   /**
    * Sets the verbosity for the specified Logger. If the verbosity
    * passed is not in the accepted range, this method will
    * fail to set the verbosity and return false. Otherwise
    * it will set the verbosity and return true.
    *
    * @param name the name of the logger.
    * @param fileVerbosity the file verbosity to set.
    * @param consoleVerbosity the console verbosity to set.
    * @return true if verbosity valid and set, false if not.
    */
   public static boolean setVerbosity(String name, double fileVerbosity,
                                      double consoleVerbosity)
   {
      boolean rval = false;

      Logger logger = getLogger(name);
      if(logger != null)
      {
         rval = logger.setFileVerbosity(fileVerbosity);
         rval &= logger.setConsoleVerbosity(consoleVerbosity);
      }

      return rval;
   }

   /**
    * Sets the file verbosity for the specified Logger. If the verbosity
    * passed is not in the accepted range, this method will
    * fail to set the verbosity and return false. Otherwise
    * it will set the verbosity and return true.
    *
    * @param name the name of the logger.
    * @param fileVerbosity the file verbosity to set.
    * @return true if file verbosity valid and set, false if not.
    */
   public static boolean setFileVerbosity(String name, double fileVerbosity)
   {
      boolean rval = false;

      Logger logger = getLogger(name);
      if(logger != null)
      {
         rval = logger.setFileVerbosity(fileVerbosity);
      }

      return rval;
   }

   /**
    * Gets the file verbosity set for the specified Logger.
    *
    * @param name the name of the logger.
    * @return the file verbosity set for the specified Logger.
    */
   public static double getFileVerbosity(String name)
   {
      double rval = -1;

      Logger logger = getLogger(name);
      if(logger != null)
      {
         rval = logger.getFileVerbosity();
      }

      return rval;
   }
   
   /**
    * Sets the console verbosity for the specified Logger. If the verbosity
    * passed is not in the accepted range, this method will
    * fail to set the verbosity and return false. Otherwise
    * it will set the verbosity and return true.
    *
    * @param name the name of the logger.
    * @param consoleVerbosity the verbosity to set.
    * @return true if console verbosity valid and set, false if not.
    */
   public static boolean setConsoleVerbosity(
      String name, double consoleVerbosity)
   {
      boolean rval = false;

      Logger logger = getLogger(name);
      if(logger != null)
      {
         rval = logger.setConsoleVerbosity(consoleVerbosity);
      }
      else
      {
         // create the logger
         if(createLogger(name))
         {
            rval = logger.setConsoleVerbosity(consoleVerbosity);
         }
      }      

      return rval;
   }

   /**
    * Gets the console verbosity set for the specified Logger.
    *
    * @param name the name of the logger.
    * @return the console verbosity set for the specified Logger.
    */
   public static double getConsoleVerbosity(String name)
   {
      double rval = -1;

      Logger logger = getLogger(name);
      if(logger != null)
      {
         rval = logger.getConsoleVerbosity();
      }

      return rval;
   }
   
   /**
    * Sets the date format for the specified logger. If the date format
    * given is not a valid format or the logger does not exist, the
    * method does nothing but return false.
    *
    * @param name the name of the logger.
    * @param dateFormat the new date format.
    * @return true if the date format is set, false if not.
    */
   public static boolean setDateFormat(String name, String dateFormat)   
   {
      boolean rval = false;

      Logger logger = getLogger(name);
      if(logger != null)
      {
         rval = logger.setDateFormat(dateFormat);
      }

      return rval;
   }

   /**
    * Uses the specified Logger. Opens a new log file with the
    * specified file name. Will append if file already exists.
    *
    * @param name the name of the logger.
    * @param filename the name of the log file.
    * @return true if succesfully opened the file for writing, false if not.
    */
   public static boolean setFile(String name, String filename)
   {
      return setFile(name, filename, true);
   }

   /**
    * Uses the specified logger. Opens a new log file with the specified
    * file name. Setting append to true will append to the
    * file if it exists. Setting it to false will overwrite it.
    *
    * @param name the name of the logger.
    * @param filename the name of the log file.
    * @param append specifies whether or not to append to an existing
    *             file or to overwrite.
    * @return true if succesfully opened the file for writing, false if not.
    */
   public static boolean setFile(String name, String filename, boolean append)
   {
      boolean rval = false;

      Logger logger = getLogger(name);
      if(logger != null)
      {
         rval = logger.setFile(filename, append);
      }
      else
      {
         // create the logger
         if(createLogger(name))
         {
            rval = setFile(name, filename, append);
         }
      }

      return rval;
   }
   
   /**
    * Resets logger files for all loggers that were using the
    * passed file (identified by file name).
    * 
    * @param filename the name of the file.
    */
   public static void resetLoggerFiles(String filename)
   {
      Iterator i = smLoggers.values().iterator();
      while(i.hasNext())
      {
         Logger logger = (Logger)i.next();
         if(logger.getFilename().equals(filename))
         {
            logger.setFile(filename, true);
         }
      }
   }
   
   /**
    * Sets the maximum log file size for a logger. Setting the maximum log file
    * size to -1 means that there is no maximum.
    * 
    * @param name the name of the logger to set.
    * @param fileSize the maximum log file size. -1 for no maximum.
    * @return true if the max file size was set, false if not.
    */
   public static boolean setMaxFileSize(String name, long fileSize)
   {
      boolean rval = false;

      Logger logger = getLogger(name);
      if(logger != null)
      {
         logger.setMaxFileSize(fileSize);
         rval = true;
      }

      return rval;      
   }
   
   /**
    * Gets the maximum log file size for a logger.
    * 
    * @param name the name of the logger.
    * @return the max log file size or -1.
    */
   public static long getMaxFileSize(String name)
   {
      long rval = -1;
      
      Logger logger = getLogger(name);
      if(logger != null)
      {
         rval = logger.getMaxFileSize();
      }
      
      return rval;
   }
   
   /**
    * Sets the number of rotating log files for a logger. This is the number of
    * files other than the main log file that may be rotated in when the
    * maximum log file size would otherwise be exceeded. No fewer than
    * 1 file may be set. If a value of less than zero is passed, then
    * there will be no limit on the number of rotating files.
    *
    * @param name the name of the logger to modify. 
    * @param numRotatingFiles the number of rotating log files.
    * @return true if successfully set, false if not.
    */
   public static boolean setNumRotatingFiles(
      String name, long numRotatingFiles)
   {
      boolean rval = false;
      
      Logger logger = getLogger(name);
      if(logger != null)
      {
         rval = logger.setNumRotatingFiles(numRotatingFiles);
      }
      
      return rval;      
   }

   /**
    * Gets the number of rotating log files for a logger. This is the number of
    * files other than the main log file that may be rotated in when the
    * maximum log file size would otherwise be exceeded.
    *
    * @param name the name of the logger to modify.
    * @return the number of rotating log files.
    */
   public static long getNumRotatingFiles(String name)
   {
      long rval = 1;
      
      Logger logger = getLogger(name);
      if(logger != null)
      {
         rval = logger.getNumRotatingFiles();
      }
      
      return rval;      
   }   
   
   /**
    * Gets the print stream for the specified logger.
    * 
    * @param name the name of the logger.
    * @return the print stream for the specified logger.
    */
   public static PrintStream getPrintStream(String name)
   {
      PrintStream ps = null;

      Logger logger = getLogger(name);
      if(logger != null)
      {
         ps = logger.getPrintStream();
      }
      
      // default to system.out
      if(ps == null)
      {
         ps = System.out;
      }

      return ps;
   }
   
   /**
    * Gets the stack trace from a throwable object.
    * 
    * @param t the throwable object.
    * @return the stack trace as a string.
    */
   public static String getStackTrace(Throwable t)
   {
      return Logger.getStackTrace(t);
   }   
}
