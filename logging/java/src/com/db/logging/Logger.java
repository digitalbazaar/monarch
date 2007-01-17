/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.logging;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.io.StringWriter;

import java.text.DateFormat;
import java.util.Date;
import java.util.Hashtable;
import java.util.Iterator;

/**
 * A class that handles logging to a file.
 *  
 * @author Dave Longley
 */
public class Logger
{
   /**
    * The name of this logger.
    */
   protected String mName;
   
   /**
    * The current verbosity setting for the log file.
    */
   protected double mFileVerbosity;
   
   /**
    * The current verbosity setting for the console.
    */
   protected double mConsoleVerbosity;

   /**
    * The date format.
    */
   protected String mDateFormat;
   
   /**
    * The filename for the log file.
    */
   protected String mFilename;

   /**
    * The file output stream to write logging information to.
    */
   protected PrintStream mStream;
   
   /**
    * The maximum file size for the log file.
    */
   protected long mMaxFileSize;
   
   /**
    * The id of the next log file to rotate out.
    */
   protected long mRotateId;
   
   /**
    * The number of rotating files. This is the number of files, other
    * than the main log file that can be rotated in/out in case a write
    * to a log file would exceed the maximum log size.
    */
   protected long mNumRotatingFiles;
   
   /**
    * A map of all of the logging print streams to their verbosities. 
    */
   protected Hashtable<PrintStream, Double> mStreamToVerbosity;
   
   /**
    * The default number of log files to rotate.
    */
   public static final long DEFAULT_NUM_ROTATING_FILES = 3;
   
   /**
    * The no verbosity setting. Nothing will be logged.
    */
   public static final double NO_VERBOSITY = 0.0;

   /**
    * The error verbosity setting. Errors will be logged.
    */
   public static final double ERROR_VERBOSITY = 1.0;

   /**
    * The warning verbosity setting. Errors and warnings will be logged.
    */
   public static final double WARNING_VERBOSITY = 2.0;

   /**
    * The message verbosity setting. Errors, warnings, and messages will be
    * logged.
    */
   public static final double MSG_VERBOSITY = 3.0;

   /**
    * The debug verbosity setting. Errors, warnings, messages, and debug
    * information will be logged.
    * 
    * Debug log output should include stack traces, etc.
    */
   public static final double DEBUG_VERBOSITY = 4.0;
   
   /**
    * The debug data verbosity setting. Errors, warnings, messages, debug,
    * and debug data will be logged.
    * 
    * Debug data log output includes any data associated with debug log output.
    */
   public static final double DEBUG_DATA_VERBOSITY = 4.1;

   /**
    * The detail verbosity setting. Errors, warnings, messages, debug,
    * and debug data information will be logged.
    * 
    * Detail log output includes very fine detailed informational messages.
    */
   public static final double DETAIL_VERBOSITY = 4.5;
   
   /**
    * The maximum verbosity setting. Everything will be logged.
    */
   public static final double MAX_VERBOSITY = 10.0;

   /**
    * Creates a new logger with default verbosity.
    * 
    * @param name the name of the logger.
    */
   public Logger(String name)
   {
      this(name, Logger.DEBUG_VERBOSITY);
   }

   /**
    * Creates a new logger with specified verbosity.
    *
    * @param name the name of the logger.
    * @param fileVerbosity the max verbosity to display in the log file.
    */
   public Logger(String name, double fileVerbosity)
   {
      this(name, fileVerbosity, Logger.NO_VERBOSITY);
   }
   
   /**
    * Creates a new logger with specified verbosity.
    *
    * @param name the name of the logger.
    * @param fileVerbosity the max verbosity to display in the log file.
    * @param consoleVerbosity the max verbosity to display in the console.
    */
   public Logger(String name, double fileVerbosity, double consoleVerbosity)
   {
      mName = name;
      mFileVerbosity = fileVerbosity;
      mConsoleVerbosity = consoleVerbosity;
      mStream = null;
      mDateFormat = "MM/dd/yy H:mm:ss";
      mMaxFileSize = -1;
      mRotateId = -1;
      mNumRotatingFiles = DEFAULT_NUM_ROTATING_FILES;
      mStreamToVerbosity = new Hashtable<PrintStream, Double>();
   }
   
   /**
    * Overloaded to ensure that the stream gets closed when garbage
    * collected.
    */
   @Override
   protected void finalize()
   {
      closeStream();
   }
   
   /**
    * Gets the current date in the appropriate format.
    * 
    * @return the current date in the appropriate format.
    */
   protected String getDate()
   {
      String date = "";

      if(mDateFormat.equals(""))
      {
         // do nothing
      }
      else
      {
         // handle other date formats here
         Date now = new Date();
         
         if(mDateFormat.equals("day mon yy hh:mm:ss yyyy"))
         {
            date = now.toString();
         }
         else if(mDateFormat.equals("MM/dd/yy H:mm:ss"))
         {
            date = DateFormat.getDateTimeInstance(
                   DateFormat.SHORT, DateFormat.LONG).format(now); 
         }
         else
         {
            date = now.toString();
         }
      }

      return date;
   }
   
   /**
    * Gets the id of the next log file to rotate out. Auto-increments for
    * the next call.
    * 
    * @return the id of the next log file to rotate out. 
    */
   protected long getRotateId()
   {
      mRotateId++;
      
      if(getNumRotatingFiles() != -1 && mRotateId >= getNumRotatingFiles())
      {
         mRotateId = 0;
      }
      
      return mRotateId;
   }
   
   /**
    * Rotates the log file as necessary. If the next append to a log file
    * would exceed its maximum size, then the log file is rotated out.
    * This method will only make changes to the log file if there has been
    * a maximum log file size set.
    * 
    * @param logText the log text to be appended to the log file.
    */
   protected synchronized void rotateLogFile(String logText)
   {
      if(getMaxFileSize() > 0)
      {
         // includes end line character
         int logTextLength = logText.length() + 1;
         
         File file = new File(mFilename);
         long newLength = file.length() + logTextLength;
         long overflow = newLength - getMaxFileSize();
         if(overflow > 0)
         {
            // there is overflow, so rotate the files
            String rotateLog = file.getAbsolutePath() + "." + getRotateId();
            File newFile = new File(rotateLog);
            
            // if there is no limit on the number of rotating files,
            // then do not overwrite existing rotated logs
            if(getNumRotatingFiles() == -1)
            {
               // keep going until an unused file is found
               while(newFile.exists())
               {
                  rotateLog = file.getAbsolutePath() + "." + getRotateId();
                  newFile = new File(rotateLog);
               }
            }
            
            // close log files temporarily
            LoggerManager.closeLoggerFiles(mFilename);
            
            // ensure the new file does not exist
            newFile.delete();
            
            // rename current log file to new file
            file.renameTo(newFile);
            
            // reset log files
            LoggerManager.resetLoggerFiles(mFilename);
         }
      }
   }
   
   /**
    * Gets the name of this logger.
    * 
    * @return the name of this logger.
    */
   public String getName()
   {
      return mName;
   }

   /**
    * Sets the file verbosity for this logger. If the verbosity
    * passed is not in the accepted range, this method will
    * fail to set the verbosity and return false. Otherwise
    * it will set the verbosity and return true.
    *
    * @param fileVerbosity the verbosity to set.
    * 
    * @return true if verbosity valid and set, false if not.
    */
   public synchronized  boolean setFileVerbosity(double fileVerbosity)
   {
      boolean rval = false;

      if(fileVerbosity > NO_VERBOSITY && fileVerbosity <= MAX_VERBOSITY)
      {
         mFileVerbosity = fileVerbosity;
         
         if(getPrintStream() != null)
         {
            if(mFileVerbosity == NO_VERBOSITY)
            {
               mStreamToVerbosity.remove(getPrintStream());
            }
            else
            {
               mStreamToVerbosity.put(getPrintStream(),
                                      new Double(mFileVerbosity));
            }
         }
         
         rval = true;
      }

      return rval;
   }

   /**
    * Gets the file verbosity set for this logger.
    *
    * @return the file verbosity set for this logger.
    */
   public double getFileVerbosity()
   {
      return mFileVerbosity;
   }
   
   /**
    * Sets the console verbosity for this logger. If the verbosity
    * passed is not in the accepted range, this method will
    * fail to set the verbosity and return false. Otherwise
    * it will set the verbosity and return true.
    *
    * @param consoleVerbosity the verbosity to set.
    * 
    * @return true if verbosity valid and set, false if not.
    */
   public synchronized boolean setConsoleVerbosity(double consoleVerbosity)
   {
      boolean rval = false;

      if(consoleVerbosity >= NO_VERBOSITY && consoleVerbosity <= MAX_VERBOSITY)
      {
         mConsoleVerbosity = consoleVerbosity;
         
         if(System.out != null)
         {
            if(mConsoleVerbosity == NO_VERBOSITY)
            {
               mStreamToVerbosity.remove(System.out);
            }
            else
            {
               mStreamToVerbosity.put(
                  System.out, new Double(mConsoleVerbosity));
            }
         }
         
         rval = true;
      }

      return rval;
   }

   /**
    * Gets the console verbosity set for this logger.
    *
    * @return the console verbosity set for this logger.
    */
   public double getConsoleVerbosity()
   {
      return mConsoleVerbosity;
   }

   /**
    * Sets the date format. If the date format given is not
    * a valid format, the method does nothing but return false.
    *
    * @param dateFormat the new date format.
    * 
    * @return true if the date format is set, false if not.
    */
   public synchronized boolean setDateFormat(String dateFormat)
   {
      boolean rval = false;

      if(dateFormat == "")
      {
         rval = true;
      }
      else
      {
         // other formats here
         mDateFormat = dateFormat;
      }

      return rval;
   }
   
   /**
    * Opens a new log file with the specified file name. Will append to
    * the file if it already exists.
    *
    * @param filename the name of the file to log to.
    * 
    * @return true if succesfully opened the file for writing, false if not.
    */
   public boolean setFile(String filename)
   {
      return setFile(filename, true);
   }   

   /**
    * Opens a new log file with the specified file name. Setting append to
    * true will append the file if it exists. Setting it to false will
    * overwrite it.
    *
    * @param filename the name of the file to log to.
    * @param append specifies whether or not to append to an existing
    *             file or to overwrite.
    *             
    * @return true if succesfully opened the file for writing, false if not.
    */
   public synchronized boolean setFile(String filename, boolean append)
   {
      boolean rval = false;
      
      closeStream();
      
      if(!append)
      {
         File file = new File(filename);
         file.delete();
      }
      
      mFilename = filename;
      
      try
      {
         FileOutputStream fos = new FileOutputStream(filename, true);
         mStream = new PrintStream(fos);
         
         // add stream to stream verbosity map
         mStreamToVerbosity.put(mStream, new Double(getFileVerbosity()));
         
         rval = true;
      }
      catch(Exception e)
      {
      }

      return rval;
   }
   
   /**
    * Closes the output stream if it is open.
    */
   public synchronized void closeStream()
   {
      if(mStream != null)
      {
         // remove stream from stream verbosity map
         mStreamToVerbosity.remove(mStream);
         
         try
         {
            mStream.close();
         }
         catch(Exception e)
         {
            // ignore
         }
         
         mStream = null;
      }
   }
   
   /**
    * Sets the maximum log file size (in bytes). Setting the maximum log file
    * size to -1 means that there is no maximum.
    * 
    * @param fileSize the maximum log file size (in bytes). -1 for no maximum.
    */
   public synchronized void setMaxFileSize(long fileSize)
   {
      // -1 is means no maximum log file size
      mMaxFileSize = Math.max(-1, fileSize);
   }
   
   /**
    * Gets the maximum log file size (in bytes).
    * 
    * @return the max log file size (in bytes).
    */
   public long getMaxFileSize()
   {
      return mMaxFileSize;
   }
   
   /**
    * Sets the number of rotating log files. This is the number of files
    * other than the main log file that may be rotated in when the
    * maximum log file size would otherwise be exceeded. No fewer than
    * 1 file may be set. If a value of less than zero is passed, then
    * there will be no limit on the number of rotating files.
    *
    * @param numRotatingFiles the number of rotating log files.
    * 
    * @return true if successfully set, false if not.
    */
   public synchronized boolean setNumRotatingFiles(long numRotatingFiles)
   {
      boolean rval = false;
      
      if(numRotatingFiles != 0)
      {
         mNumRotatingFiles = Math.max(-1, numRotatingFiles);
         rval = true;
      }
      
      return rval;
   }

   /**
    * Gets the number of rotating log files. This is the number of files
    * other than the main log file that may be rotated in when the
    * maximum log file size would otherwise be exceeded.
    *
    * @return the number of rotating log files.
    */
   public long getNumRotatingFiles()
   {
      return mNumRotatingFiles;
   }
   
   /**
    * Gets the filename set for this logger.
    * 
    * @return the filename set for this logger.
    */
   public String getFilename()
   {
      return mFilename;
   }
   
   /**
    * Gets the print stream for this logger.
    * 
    * @return the print stream for this logger.
    */
   public PrintStream getPrintStream()
   {
      return mStream;
   }
   
   /**
    * Gets a print writer for this logger.
    * 
    * @return a print writer for this logger.
    */
   public PrintWriter getPrintWriter()
   {
      PrintWriter pw = null;
      
      try
      {
         pw = new PrintWriter(getPrintStream());
      }
      catch(Throwable t)
      {
      }
      
      return pw;
   }   
   
   /**
    * Adds a print stream to the logger.
    * 
    * @param ps the print stream to add.
    * @param verbosity the verbosity for the print stream.
    */
   public synchronized void addPrintStream(PrintStream ps, double verbosity)
   {
      setPrintStreamVerbosity(ps, verbosity);
   }
   
   /**
    * Removes a print stream from the logger.
    * 
    * @param ps the print stream to remove.
    */
   public synchronized void removePrintStream(PrintStream ps)
   {
      if(ps != null)
      {
         mStreamToVerbosity.remove(ps);
      }
   }
   
   /**
    * Sets a print stream's verbosity.
    * 
    * @param ps the print stream to modify.
    * @param verbosity the verbosity for the print stream.
    */
   public synchronized void setPrintStreamVerbosity(
      PrintStream ps, double verbosity)
   {
      if(ps != null)
      {
         if(ps == getPrintStream())
         {
            setFileVerbosity(verbosity);
         }
         else if(ps == System.out)
         {
            setConsoleVerbosity(verbosity);
         }
         else
         {
            mStreamToVerbosity.put(ps, new Double(verbosity));
         }
      }
   }
   
   /**
    * Writes the passed string to this logger.
    *
    * @param text the text to write to this logger.
    * 
    * @return true if the text was written, false if not.
    */
   public boolean log(String text)
   {
      return log(null, text);
   }

   /**
    * Writes the passed string to this logger for the given class.
    *
    * @param c the class to write to this logger for.
    * @param text the text to write to this logger.
    * 
    * @return true if the text was written, false if not.
    */
   public boolean log(Class c, String text)
   {
      return log(c, text, Logger.ERROR_VERBOSITY);
   }
   
   /**
    * Writes the passed string to the console/log file, if it is open.
    *
    * @param c the class to write to this logger for.
    * @param text the text to write to the log file.
    * @param verbosity the verbosity level that must be reached in
    *                  order for the text to be written to the log.
    * 
    * @return true if the text was written, false if not.
    */
   public synchronized boolean log(Class c, String text, double verbosity)
   {
      return log(c, text, verbosity, true, true);
   }
   
   /**
    * Writes the passed string to the console/log file, if it is open.
    *
    * @param c the class to write to this logger for.
    * @param text the text to write to the log file.
    * @param verbosity the verbosity level that must be reached in
    *                  order for the text to be written to the log.
    * @param header true to use the logger's header, false not to.
    * @param useCustomStreams true to print to custom streams, false to only
    *                         print to console/log file.
    * 
    * @return true if the text was written, false if not.
    */
   public synchronized boolean log(
      Class c, String text, double verbosity,
      boolean header, boolean useCustomStreams)
   {
      boolean rval = false;
      
      if(mStreamToVerbosity.size() != 0)
      {
         String logText = "";
         
         if(header)
         {
            String date = getDate();
            if(!date.equals(""))
            {
               logText = getDate() + " ";
            }
            
            logText += getName() + " ";
            
            if(c != null)
            {
               logText += c.getSimpleName() + " - ";
            }
            else
            {
               logText += "- ";
            }
            
            logText += text;
         }
         else
         {
            logText = text;
         }
         
         String logFileText = logText;
         
         // if entire log text cannot be entered, break it up
         String remainder = "";
         if(getMaxFileSize() > 0 &&
            (logText.length() + 1) > getMaxFileSize())
         {
            remainder = logText.substring((int)getMaxFileSize());
            logFileText = logText.substring(0, ((int)getMaxFileSize()) - 1);
         }
         
         // lock on the loggermanager
         synchronized(LoggerManager.getInstance())
         {
            // ensure a file is set if appropriate
            if(getMaxFileSize() != 0 &&
               mFilename != null && !mFilename.equals(""))
            {
               File file = new File(mFilename);
               
               // if the file no longer exists, start a new file
               if(!file.exists())
               {
                  LoggerManager.resetLoggerFiles(mFilename);
               }
            }
            
            // rotate the log file if necessary
            rotateLogFile(logFileText);
            
            // print to all appropriate streams
            Iterator i = mStreamToVerbosity.keySet().iterator();
            while(i.hasNext())
            {
               // get the next stream and its verbosity
               PrintStream ps = (PrintStream)i.next();
               Double sv = mStreamToVerbosity.get(ps);
               
               if(sv != null)
               {
                  if(sv.doubleValue() >= verbosity)
                  {
                     try
                     {
                        if(ps == getPrintStream())
                        {
                           ps.println(logFileText);
                        }
                        else
                        {
                           if(ps == System.out || useCustomStreams)
                           {
                              ps.println(logText);
                           }
                        }
                        
                        ps.flush();
                     }
                     catch(Throwable t)
                     {
                        // there was an error with the stream, remove it
                        // from the stream verbosity map
                        i.remove();
                     }
                  }
               }
            }
            
            // if there is any remainder, log it without a logger header
            if(!remainder.equals(""))
            {
               log(c, remainder, verbosity, false, false);
            }
            
            rval = true;
         }
      }

      return rval;
   }
   
   /**
    * Gets the stack trace from a throwable object.
    * 
    * @param t the throwable object.
    * 
    * @return the stack trace as a string.
    */
   public static String getStackTrace(Throwable t)
   {
      String rval = "null";
      
      if(t != null)
      {
         StringWriter sw = new StringWriter();
         PrintWriter pw = new PrintWriter(sw);
         t.printStackTrace(pw);
         pw.close();
         
         rval = sw.toString();
      }
      
      return rval;
   }   
   
   /**
    * Writes the passed string to this logger.
    * Verbosity is set to error level.
    *
    * @param text the text to write to this logger.
    * 
    * @return true if the text was written, false if not.
    */
   public boolean error(String text)
   {
      return error(null, text);
   }

   /**
    * Writes the passed string to this logger for the given class.
    * Verbosity is set to error level.
    *
    * @param c the class to write to the logger for.
    * @param text the text to write to the logger.
    * 
    * @return true if the text was written, false if not.
    */
   public boolean error(Class c, String text)
   {
      boolean rval = false;
      
      rval = log(c, "ERROR: " + text, ERROR_VERBOSITY);
      
      return rval;
   }
   
   /**
    * Writes the passed string to this logger.
    * Verbosity is set to warning level.
    *
    * @param text the text to write to this logger.
    * 
    * @return true if the text was written, false if not.
    */
   public boolean warning(String text)
   {
      return warning(null, text);
   }

   /**
    * Writes the passed string to this logger for the given class.
    * Verbosity is set to warning level.
    *
    * @param c the class to write for.
    * @param text the text to write to this logger.
    * 
    * @return true if the text was written, false if not.
    */
   public boolean warning(Class c, String text)
   {
      boolean rval = false;
      
      rval = log(c, "WARNING: " + text, WARNING_VERBOSITY);
      
      return rval;
   }

   /**
    * Writes the passed string to this logger.
    * Verbosity is set to message level.
    *
    * @param text the text to write to the logger.
    * 
    * @return true if the text was written, false if not.
    */
   public boolean msg(String text)
   {
      return msg(null, text);
   }

   /**
    * Writes the passed string to this logger for the given class.
    * Verbosity is set to message level.
    *
    * @param c the class to write to this logger for.
    * @param text the text to write to this logger.
    * 
    * @return true if the text was written, false if not.
    */
   public boolean msg(Class c, String text)
   {
      boolean rval;
      
      rval = log(c, "MSG: " + text, MSG_VERBOSITY);
      
      return rval;
   }
   
   /**
    * Writes the passed string to this logger.
    * Verbosity is set to debug level.
    *
    * @param text the text to write to this logger.
    * 
    * @return true if the text was written, false if not.
    */
   public boolean debug(String text)
   {
      return debug(null, text);
   }

   /**
    * Writes the passed string to this logger for the given class.
    * Verbosity is set to debug level.
    *
    * @param c the class to write to this logger for.
    * @param text the text to write to this logger.
    * 
    * @return true if the text was written, false if not.
    */
   public boolean debug(Class c, String text)
   {
      boolean rval = false;
      
      rval = log(c, "DEBUG: " + text, DEBUG_VERBOSITY);
      
      return rval;
   }
   
   /**
    * Writes the passed string to this logger.
    * Verbosity is set to debug data level.
    *
    * @param text the text to write to this logger.
    * 
    * @return true if the text was written, false if not.
    */
   public boolean debugData(String text)
   {
      return debugData(null, text);
   }

   /**
    * Writes the passed string to this logger for the given class.
    * Verbosity is set to debug data level.
    *
    * @param c the class to write to this logger for.
    * @param text the text to write to this logger.
    * 
    * @return true if the text was written, false if not.
    */
   public boolean debugData(Class c, String text)
   {
      boolean rval = false;
      
      rval = log(c, "DEBUG-DATA: " + text, DEBUG_DATA_VERBOSITY);
      
      return rval;
   }
   
   /**
    * Writes the passed string to this logger.
    * Verbosity is set to detail level.
    *
    * @param text the text to write to the log file.
    * 
    * @return true if the text was written, false if not.
    */
   public boolean detail(String text)
   {
      return detail(null, text);
   }
   
   /**
    * Writes the passed string to this logger for the given class.
    * Verbosity is set to detail level.
    *
    * @param c the class to write to this logger for.
    * @param text the text to write to the log file.
    * 
    * @return true if the text was written, false if not.
    */
   public boolean detail(Class c, String text)
   {
      boolean rval = false;
      
      rval = log(c, "DETAIL: " + text, DETAIL_VERBOSITY);
      
      return rval;
   }
}
