/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Collections;
using System.Globalization;
using System.IO;
using System.Runtime.CompilerServices;
using System.Text;

namespace DB.Common.Logging
{
   /// <summary>
   /// A class that handles logging to a file.
   /// </summary>
   ///
   /// <author>Dave Longley</author>
   public class Logger : IDisposable
   {
      /// <summary>
      /// The name of this logger.
      /// </summary>
      protected string mName;
   
      /// <summary>
      /// The current verbosity setting for the log file.
      /// </summary>
      protected int mFileVerbosity;
   
      /// <summary>
      /// The current verbosity setting for the console.
      /// </summary>
      protected int mConsoleVerbosity;

      /// <summary>
      /// The date format.
      /// </summary>
      protected string mDateFormat;
   
      /// <summary>
      /// The filename for the log file.
      /// </summary>
      protected string mFilename;

      /// <summary>
      /// The primary text writer to write logging information to.
      /// </summary>
      protected TextWriter mTextWriter;
   
      /// <summary>
      /// The maximum file size for the log file.
      /// </summary>
      protected long mMaxFileSize;
   
      /// <summary>
      /// The id of the next log file to rotate out.
      /// </summary>
      protected long mRotateId;
   
      /// <summary>
      /// The number of rotating files. This is the number of files, other
      /// than the main log file that can be rotated in/out in case a write
      /// to a log file would exceed the maximum log size.
      /// </summary>
      protected long mNumRotatingFiles;
      
      /// <summary>
      /// A map of all of the logging text writers to their verbosities.
      /// </summary>
      protected Hashtable mWriterToVerbosity;
      
      /// <summary>
      /// Whether or not this logger has been disposed yet.
      /// </summary>
      protected bool mDisposed;
      
      /// <summary>
      /// The default number of log files to rotate.
      /// </summary>
      public static readonly long DEFAULT_NUM_ROTATING_FILES = 3;
      
      public static readonly int NO_VERBOSITY = 0;
      public static readonly int ERROR_VERBOSITY = 1;
      public static readonly int WARNING_VERBOSITY = 2;
      public static readonly int MSG_VERBOSITY = 3;
      public static readonly int DEBUG_VERBOSITY = 4;
      public static readonly int DEBUG_DATA_VERBOSITY = 5;
      public static readonly int MAX_VERBOSITY = DEBUG_DATA_VERBOSITY;
      
      /// <summary>
      /// Creates a new logger with default verbosity.
      /// </summary>
      ///
      /// <param name="name">the name of the logger.</param>
      public Logger(string name) :
         this(name, DEBUG_VERBOSITY)
      {
      }

      /// <summary>
      /// Creates a new logger with specified verbosity.
      /// </summary>
      ///
      /// <param name="name">the name of the logger.</param>
      /// <param name="fileVerbosity">the max verbosity to display in the
      /// log file.</param>
      public Logger(string name, int fileVerbosity) :
         this(name, fileVerbosity, NO_VERBOSITY)
      {
      }
      
      /// <summary>
      /// Creates a new logger with specified verbosity.
      /// </summary>
      ///
      /// <param name="name">the name of the logger.</param>
      /// <param name="fileVerbosity">the max verbosity to display in the
      /// log file.</param>
      /// <param name="consoleVerbosity">the max verbosity to display in the
      /// console.</param>
      public Logger(string name, int fileVerbosity, int consoleVerbosity)
      {
         mName = name;
         mFileVerbosity = fileVerbosity;
         mConsoleVerbosity = consoleVerbosity;
         mTextWriter = null;
         mDateFormat = "M/d/yyyy h:mm:ss tt 'GMT' z";
         mMaxFileSize = -1;
         mRotateId = -1;
         mNumRotatingFiles = DEFAULT_NUM_ROTATING_FILES;
         mWriterToVerbosity = new Hashtable();
         mDisposed = false;
      }
      
      /// <summary>
      /// Ensures that the text writer gets closed.
      /// </summary>
      protected virtual void Dispose(bool disposing)
      {
         lock(this)
         {
            if(!mDisposed && disposing)
            {
               CloseStream();
               GC.SuppressFinalize(this);
               mDisposed = true;
            }
         }
      }

      /// <summary>
      /// Closes the text writer if it is open.
      /// </summary>
      [MethodImpl(MethodImplOptions.Synchronized)]
      protected virtual void CloseStream()
      {
         if(TextWriter != null)
         {
            // remove stream from stream verbosity map
            mWriterToVerbosity.Remove(TextWriter);
            
            try
            {
               TextWriter.Close();
            }
            catch
            {
            }
            
            mTextWriter = null;
         }
      }

      /// <summary>
      /// Gets the current date in the appropriate format.
      /// </summary>
      ///
      /// <returns>the current date in the appropriate format.</returns>
      protected virtual string Date
      {
         get
         {
            string date = "";

            if(DateFormat.Equals(""))
            {
               // do nothing
            }
            else
            {
               DateTime now = DateTime.Now;
               
               date = now.ToString(
                  DateFormat, DateTimeFormatInfo.InvariantInfo);
            }
            
            return date;
         }
      }
      
      /// <summary>
      /// Gets the id of the next log file to rotate out. Auto-increments for
      /// the next call.
      /// </summary>
      ///
      /// <returns>the id of the next log file to rotate out.</returns>
      protected virtual long RotateId
      {
         get
         {
            mRotateId++;
         
            if(NumRotatingFiles != -1 && mRotateId > NumRotatingFiles)
            {
               mRotateId = 0;
            }
         
            return mRotateId;
         }
      }
      
      /// <summary>
      /// Rotates the log file as necessary. If the next append to a log file
      /// would exceed its maximum size, then the log file is rotated out.
      ///
      /// This method will only make changes to the log file if there has been
      /// a maximum log file size set.
      /// </summary>
      ///
      /// <param name="logText">log text to be appended to the log file.</param>
      [MethodImpl(MethodImplOptions.Synchronized)]
      protected virtual void RotateLogFile(string logText)
      {
         if(MaxFileSize > 0)
         {
            // includes end line character
            int logTextLength = logText.Length + 1;
            
            FileInfo fileInfo = new FileInfo(Filename);
            long newLength = fileInfo.Length + logTextLength;
            long overflow = newLength - MaxFileSize;
            if(overflow > 0)
            {
               // there is overflow, so rotate the files
               string rotateLog = fileInfo.FullName + "." + RotateId;
               FileInfo newFileInfo = new FileInfo(rotateLog);

               // if there is no limit on the number of rotating files,
               // then do not overwrite existing rotated logs
               if(NumRotatingFiles == -1)
               {
                  // keep going until an unused file is found
                  while(newFileInfo.Exists)
                  {
                     rotateLog = fileInfo.FullName + "." + RotateId;
                     newFileInfo = new FileInfo(rotateLog);
                  }
               }
             
               // rename current log file to new file
               fileInfo.MoveTo(rotateLog);
            
               // reset the log files
               LoggerManager.ResetLoggerFiles(Filename);
            }
         }
      }
      
      /// <summary>
      /// Disposes this logger.
      /// </summary>
      public virtual void Dispose()
      {
         if(!mDisposed)
         {
            Dispose(true);
         }
      }
      
      /// <summary>
      /// Gets the name of this logger.
      /// </summary>
      ///
      /// <returns>the name of this logger.</returns>
      public virtual string Name
      {
         get
         {
            return mName;
         }
      }

      /// <summary>
      /// Gets/Sets the file verbosity for this logger. If the verbosity
      /// passed is not in the accepted range, it will not be set.
      /// </summary>
      public virtual int FileVerbosity
      {
         get
         {
            return mFileVerbosity;
         }
         
         set
         {
            lock(this)
            {
               if(value > NO_VERBOSITY && value <= MAX_VERBOSITY)
               {
                  mFileVerbosity = value;
            
                  if(TextWriter != null)
                  {
                     if(FileVerbosity == NO_VERBOSITY)
                     {
                        mWriterToVerbosity.Remove(TextWriter);
                     }
                     else
                     {
                        mWriterToVerbosity[TextWriter] = FileVerbosity;
                     }
                  }
               }
            }
         }
      }
      
      /// <summary>
      /// Gets/Sets the console verbosity for this logger. If the verbosity
      /// passed is not in the accepted range, the verbosity will not be set.
      /// </summary>
      public virtual int ConsoleVerbosity
      {
         get
         {
            return mConsoleVerbosity;
         }
         
         set
         {
            lock(this)
            {
               if(value >= NO_VERBOSITY && value <= MAX_VERBOSITY)
               {
                  mConsoleVerbosity = value;
               
                  if(Console.Out != null)
                  {
                     if(ConsoleVerbosity == NO_VERBOSITY)
                     {
                        mWriterToVerbosity.Remove(Console.Out);
                     }
                     else
                     {
                        mWriterToVerbosity[Console.Out] = ConsoleVerbosity;
                     }
                  }
               }
            }
         }
      }

      /// <summary>
      /// Sets the date format. If the date format given is not
      /// a valid format it will not be set.
      /// </summary>
      public virtual string DateFormat
      {
         get
         {
            return mDateFormat;
         }
      
         set
         {
            lock(this)
            {
               if(value != null && !value.Equals(""))
               {
                  mDateFormat = value;
               }
            }
         }
      }
      
      /// <summary>
      /// Opens a new log file with the specified file name. Will append to
      /// the file if it already exists.
      /// </summary>
      ///
      /// <param name="filename">the name of the file to log to.</param>
      /// <returns>true if succesfully opened the file for writing,
      /// false if not.</returns>
      public virtual bool SetFile(string filename)
      {
         return SetFile(filename, true);
      }   

      /// <summary>
      /// Opens a new log file with the specified file name. Setting append to
      /// true will append the file if it exists. Setting it to false will
      /// overwrite it.
      /// </summary>
      ///
      /// <param name="filename">the name of the file to log to.</param>
      /// <param name="append">specifies whether or not to append to an existing
      /// file or to overwrite.</param>
      /// <returns>true if succesfully opened the file for writing, false
      /// if not.</returns>
      [MethodImpl(MethodImplOptions.Synchronized)]
      public virtual bool SetFile(string filename, bool append)
      {
         bool rval = false;
         
         CloseStream();
         
         if(!append)
         {
            FileInfo fileInfo = new FileInfo(filename);
            fileInfo.Delete();
         }
         
         mFilename = filename;
         
         try
         {
            if(append)
            {
               FileStream fs = new FileStream(filename,
                                              FileMode.Append,
                                              FileAccess.Write,
                                              FileShare.ReadWrite);
               mTextWriter = new StreamWriter(fs);
            }
            else
            {
               FileStream fs = new FileStream(filename,
                                              FileMode.Create,
                                              FileAccess.Write,
                                              FileShare.ReadWrite);
               mTextWriter = new StreamWriter(fs);
            }
            
            // add stream to stream verbosity map
            mWriterToVerbosity[TextWriter] = FileVerbosity;
          
            rval = true;
         }
         catch
         {
         }

         return rval;
      }
      
      /// <summary>
      /// Gets the filename set for this logger.
      /// </summary>
      ///
      /// <returns>the filename set for this logger.</returns>
      public virtual string Filename
      {
         get
         {
            return mFilename;
         }
      }      
      
      /// <summary>
      /// Gets/Sets the maximum log file size. Setting the maximum log file
      /// size to -1 means that there is no maximum.
      /// </summary>
      public virtual long MaxFileSize
      {
         get
         {
            return mMaxFileSize;
         }
         
         set
         {
            lock(this)
            {
               // -1 is means no maximum log file size
               mMaxFileSize = Math.Max(-1, value);
            }
         }
      }
      
      /// <summary>
      /// Gets/Sets the number of rotating log files. This is the number of
      /// files other than the main log file that may be rotated in when the
      /// maximum log file size would otherwise be exceeded. No fewer than
      /// 1 file may be set. If a value of less than zero is passed, then
      /// there will be no limit on the number of rotating files.
      /// </summary>
      public virtual long NumRotatingFiles
      {
         get
         {
            return mNumRotatingFiles;
         }
         
         set
         {
            if(value != 0)
            {
               mNumRotatingFiles = Math.Max(-1, value);
            }
         }
      }
      
      /// <summary>
      /// Gets the text writer for this logger.
      /// </summary>
      ///
      /// <returns>the text writer for this logger.</returns>
      public virtual TextWriter TextWriter
      {
         get
         {
            return mTextWriter;
         }
      }
      
      /// <summary>
      /// Adds a text writer to this logger.
      /// </summary>
      ///
      /// <param name="tw">the text writer to add.</param>
      /// <param name="verbosity">the verbosity for the text writer.</param>
      [MethodImpl(MethodImplOptions.Synchronized)]
      public virtual void AddTextWriter(TextWriter tw, int verbosity)
      {
         SetTextWriterVerbosity(tw, verbosity);
      }
      
      /// <summary>
      /// Removes a text writer from this logger.
      /// </summary>
      ///
      /// <param name="tw">the text writer to remove.</param>
      [MethodImpl(MethodImplOptions.Synchronized)]
      public virtual void RemoveTextWriter(TextWriter tw)
      {
         if(tw != null)
         {
            mWriterToVerbosity.Remove(tw);
         }
      }
      
      /// <summary>
      /// Sets a text writer's verbosity.
      /// </summary>
      ///
      /// <param name="tw">the text writer to modify.</param>
      /// <param name="verbosity">the verbosity for the text writer.</param>
      [MethodImpl(MethodImplOptions.Synchronized)]
      public virtual void SetTextWriterVerbosity(TextWriter tw, int verbosity)
      {
         if(tw != null)
         {
            if(tw == TextWriter)
            {
               FileVerbosity = verbosity;
            }
            else if(tw == Console.Out)
            {
               ConsoleVerbosity = verbosity;
            }
            else
            {
               mWriterToVerbosity[tw] = verbosity;
            }
         }
      }
      
      /// <summary>
      /// Writes the passed string to the log file, if it is open.
      /// </summary>
      ///
      /// <param name="text">the text to write to the log file.</param>
      /// <returns>true if the text was written, false if not.</returns>
      public virtual bool Log(string text)
      {
         return Log(text, Logger.ERROR_VERBOSITY);
      }

      /// <summary>
      /// Writes the passed string to the console/log file, if it is open.
      /// </summary>
      ///
      /// <param name="text">the text to write to the log file.</param>
      /// <param name="verbosity">the verbosity level that must be reached in
      /// order for the text to be written to the log.</param>
      /// <returns>true if the text was written, false if not.</returns>
      [MethodImpl(MethodImplOptions.Synchronized)]
      public virtual bool Log(string text, int verbosity)
      {
         return Log(text, verbosity, true, true);
      }
      
      /// <summary>
      /// Writes the passed string to the console/log file, if it is open.
      /// </summary>
      ///
      /// <param name="text">the text to write to the log file.</param>
      /// <param name="verbosity">the verbosity level that must be reached in
      /// order for the text to be written to the log.</param>
      /// <param name="header">true to use the logger's header, false not
      /// to.</param>
      /// <param name="useCustomWriters">true to write to custom writers,
      /// false to only write to console/log file.</param>
      /// <returns>true if the text was written, false if not.</returns>
      [MethodImpl(MethodImplOptions.Synchronized)]
      public virtual bool Log(string text, int verbosity, bool header,
                              bool useCustomWriters)
      {
         bool rval = false;
         
         if(mWriterToVerbosity.Count != 0)
         {
            string logText = ""; 
               
            if(header)
            {
               string date = Date;
               if(!date.Equals(""))
               {
                  logText = Date + " " + Name + " - " + text;
               }
               else
               {
                  logText = Name + " - " + text;
               }
            }
            else
            {
               logText = text;
            }

            string logFileText = logText;

            // if entire log text cannot be entered, break it up
            string remainder = "";
            if(MaxFileSize > 0 && (logText.Length + 1) > MaxFileSize)
            {
               remainder = logText.Substring((int)MaxFileSize);
               logFileText = logText.Substring(0, ((int)MaxFileSize) - 1);
            }

            // lock on the loggermanager
            lock(LoggerManager.Instance)
            {
               // ensure a file is set if appropriate
               if(MaxFileSize != 0 &&
                  Filename != null && !Filename.Equals(""))
               {
                  FileInfo fileInfo = new FileInfo(Filename);
                  
                  // if the file no longer exists, start a new file
                  if(!fileInfo.Exists)
                  {
                     LoggerManager.ResetLoggerFiles(Filename);
                  }
               }

               // rotate log file if necessary
               RotateLogFile(logFileText);
               
               // keep track of invalid text writers
               ArrayList removeList = new ArrayList();
               
               // print to all appropriate writers
               foreach(TextWriter tw in mWriterToVerbosity.Keys)
               {
                  try
                  {
                     int v = (int)mWriterToVerbosity[tw];
                  
                     if(v >= verbosity)
                     {
                        try
                        {
                           if(tw is StreamWriter)
                           {
                              // move to the end of the stream
                              StreamWriter sw = tw as StreamWriter;
                              sw.BaseStream.Seek(0, SeekOrigin.End);
                           }
                           
                           if(tw == TextWriter)
                           {
                              tw.WriteLine(logFileText);
                           }
                           else
                           {
                              if(tw == Console.Out || useCustomWriters)
                              {
                                 tw.WriteLine(logText);
                              }
                           }
                           
                           tw.Flush();
                        }
                        catch
                        {
                           // there was an error with the writer, add it
                           // to the remove list
                           removeList.Add(tw);
                        }
                     }
                  }
                  catch
                  {
                  }
               }
               
               // remove invalid text writers
               foreach(TextWriter tw in removeList)
               {
                  mWriterToVerbosity.Remove(tw);
               }

               // if there is any remainder, log it without a logger header
               if(remainder != "")
               {
                  Log(remainder, verbosity, false, false);
               }
               
               rval = true;
            }
         }

         return rval;
      }
      
      /// <summary>
      /// Writes the passed string to the log file, if it is open.
      /// Verbosity is set to error level.
      /// </summary>
      ///
      /// <param name="text">the text to write to the log file.</param>
      /// <returns>true if the text was written, false if not.</returns>
      public virtual bool Error(string text)
      {
         bool rval = false;
         
         rval = Log("ERROR: " + text, ERROR_VERBOSITY);
         
         return rval;
      }

      /// <summary>
      /// Writes the passed string to the log file, if it is open.
      /// Verbosity is set to warning level.
      /// </summary>
      ///
      /// <param name="text">the text to write to the log file.</param>
      /// <returns>true if the text was written, false if not.</returns>
      public virtual bool Warning(string text)
      {
         bool rval = false;
         
         rval = Log("WARNING: " + text, WARNING_VERBOSITY);
         
         return rval;
      }

      /// <summary>
      /// Writes the passed string to the log file, if it is open.
      /// Verbosity is set to debug level.
      /// </summary>
      ///
      /// <param name="text">the text to write to the log file.</param>
      /// <returns>true if the text was written, false if not.</returns>
      public virtual bool Debug(string text)
      {
         bool rval = false;
         
         rval = Log("DEBUG: " + text, DEBUG_VERBOSITY);
         
         return rval;
      }

      /// <summary>
      /// Writes the passed string to the log file, if it is open.
      /// Verbosity is set to data debug level.
      /// </summary>
      ///
      /// <param name="text">the text to write to the log file.</param>
      /// <returns>true if the text was written, false if not.</returns>
      public virtual bool DebugData(string text)
      {
         bool rval = false;
         
         rval = Log("DEBUG DATA: " + text, DEBUG_DATA_VERBOSITY);
         
         return rval;
      }
      
      /// <summary>
      /// Writes the passed string to the log file, if it is open.
      /// Verbosity is set to message level.
      /// </summary>
      ///
      /// <param name="text">the text to write to the log file.</param>
      /// <returns>true if the text was written, false if not.</returns>
      public virtual bool Msg(string text)
      {
         bool rval;
         
         rval = Log("MSG: " + text, MSG_VERBOSITY);
         
         return rval;
      }
   }
}
