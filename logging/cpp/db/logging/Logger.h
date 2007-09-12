/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_logging_Logger_H
#define db_logging_Logger_H

#include <map>

#include "db/rt/Object.h"
#include "db/io/File.h"
#include "db/io/OutputStream.h"

using namespace std;
using namespace db::io;

namespace db
{
namespace logging
{

/**
 * A class that handles logging to a file.
 *  
 * @author Dave Longley
 * @author David I. Lehn
 */
class Logger : public virtual db::rt::Object
{
public:
   typedef enum Level {
      /**
       * The no level setting. Nothing will be logged.
       */
      None = 0,
      /**
       * The error level setting. Errors will be logged.
       */
      Error,
      /**
       * The warning level setting. Errors and warnings will be logged.
       */
      Warning,
      /**
       * The message level setting. Errors, warnings, and messages will be
       * logged.
       */
      Message,
      /**
       * The debug level setting. Errors, warnings, messages, and debug
       * information will be logged.
       * 
       * Debug log output should include stack traces, etc.
       */
      Debug,
      /**
       * The debug data level setting. Errors, warnings, messages, debug,
       * and debug data will be logged.
       * 
       * Debug data log output includes any data associated with debug log output.
       */
      DebugData,
      /**
       * The detail level setting. Errors, warnings, messages, debug,
       * and debug data information will be logged.
       * 
       * Detail log output includes very fine detailed informational messages.
       */
      Detail,
      /**
       * The maximum level setting. Everything will be logged.
       */
      Max
   };

protected:
   /**
    * The name of this logger.
    */
   const char* mName;
   
   /**
    * The current level setting for the log file.
    */
   Level mFileLevel;
   
   /**
    * The current level setting for the console.
    */
   Level mConsoleLevel;
   
   /**
    * The date format.
    */
   const char* mDateFormat;
   
   /**
    * The file for the log file.
    */
   File* mFile;

   /**
    * The file output stream to write logging information to.
    */
   OutputStream* mStream;
   
   /**
    * The maximum file size for the log file.
    */
   off_t mMaxFileSize;
   
   /**
    * The id of the next log file to rotate out.
    */
   long mRotateId;
   
   /**
    * The number of rotating files. This is the number of files, other
    * than the main log file that can be rotated in/out in case a write
    * to a log file would exceed the maximum log size.
    */
   long mNumRotatingFiles;
   
   /**
    * A map of all of the logging print streams to their levels. 
    */
   map<OutputStream*, Level> mStreamToLevel;
   
public:
   /**
    * The default number of log files to rotate.
    */
   static int DEFAULT_NUM_ROTATING_FILES;
   
   /**
    * Creates a new logger with specified level.
    *
    * @param name the name of the logger.
    * @param fileLevel the max level to display in the log file.
    * @param consoleLevel the max level to display in the console.
    */
   Logger(const char* name, Level fileLevel = Debug, Level consoleLevel = None);
   
   /**
    * Overloaded to ensure that the stream gets closed when garbage
    * collected.
    */
   virtual ~Logger();
   
   /**
    * Gets the current date in the appropriate format.
    * 
    * @return the current date in the appropriate format.
    */
   virtual const char* getDate();
   
   /**
    * Gets the id of the next log file to rotate out. Auto-increments for
    * the next call.
    * 
    * @return the id of the next log file to rotate out. 
    */
   virtual long getRotateId();
   
   /**
    * Rotates the log file as necessary. If the next append to a log file
    * would exceed its maximum size, then the log file is rotated out.
    * This method will only make changes to the log file if there has been
    * a maximum log file size set.
    * 
    * @param logText the log text to be appended to the log file.
    */
   virtual void rotateLogFile(const char* logText);
   
   /**
    * Gets the name of this logger.
    * 
    * @return the name of this logger.
    */
   virtual const char* getName();

   /**
    * Sets the file level for this logger. If the level
    * passed is not in the accepted range, this method will
    * fail to set the level and return false. Otherwise
    * it will set the level and return true.
    *
    * @param fileLevel the level to set.
    * 
    * @return true if level valid and set, false if not.
    */
   virtual bool setFileLevel(Level fileLevel);

   /**
    * Gets the file level set for this logger.
    *
    * @return the file level set for this logger.
    */
   virtual Level getFileLevel();
   
   /**
    * Sets the console level for this logger. If the level
    * passed is not in the accepted range, this method will
    * fail to set the level and return false. Otherwise
    * it will set the level and return true.
    *
    * @param consoleLevel the level to set.
    * 
    * @return true if level valid and set, false if not.
    */
   virtual bool setConsoleLevel(Level consoleLevel);

   /**
    * Gets the console level set for this logger.
    *
    * @return the console level set for this logger.
    */
   virtual Level getConsoleLevel();

   /**
    * Sets the date format. If the date format given is not
    * a valid format, the method does nothing but return false.
    *
    * @param dateFormat the new date format.
    * 
    * @return true if the date format is set, false if not.
    */
   virtual bool setDateFormat(const char* dateFormat);
   
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
   virtual bool setFile(const char* filename, bool append = true);
   
   /**
    * Closes the output stream if it is open.
    */
   virtual void closeStream();
   
   /**
    * Sets the maximum log file size (in bytes). Setting the maximum log file
    * size to 0 means that there is no maximum.
    * 
    * @param fileSize the maximum log file size (in bytes) or 0 for no maximum.
    */
   virtual void setMaxFileSize(off_t fileSize);
   
   /**
    * Gets the maximum log file size (in bytes).
    * 
    * @return the max log file size (in bytes) or 0 for no maximum.
    */
   virtual off_t getMaxFileSize();
   
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
   virtual bool setNumRotatingFiles(long numRotatingFiles);

   /**
    * Gets the number of rotating log files. This is the number of files
    * other than the main log file that may be rotated in when the
    * maximum log file size would otherwise be exceeded.
    *
    * @return the number of rotating log files.
    */
   virtual long getNumRotatingFiles();
   
   /**
    * Gets the filename set for this logger.
    * 
    * @return the filename set for this logger.
    */
   virtual File* getFile();
   
   /**
    * Gets the print stream for this logger.
    * 
    * @return the print stream for this logger.
    */
   virtual OutputStream* getOutputStream();
   
   /**
    * Gets a print writer for this logger.
    * 
    * @return a print writer for this logger.
    */
   //PrintWriter* getPrintWriter();
   
   /**
    * Adds a print stream to the logger.
    * 
    * @param os the print stream to add.
    * @param level the level for the print stream.
    */
   virtual void addOutputStream(OutputStream* os, Level level);
   
   /**
    * Removes a print stream from the logger.
    * 
    * @param os the print stream to remove.
    */
   virtual void removeOutputStream(OutputStream* os);
   
   /**
    * Sets a print stream's level.
    * 
    * @param os the print stream to modify.
    * @param level the level for the print stream.
    */
   virtual void setOutputStreamLevel(OutputStream* os, Level level);
   
   /**
    * Writes the passed string to the console/log file, if it is open.
    *
    * @param text the text to write to the log file.
    * @param c the class to write to this logger for.
    * @param level the level level that must be reached in
    *                  order for the text to be written to the log.
    * @param header true to use the logger's header, false not to.
    * @param useCustomStreams true to print to custom streams, false to only
    *                         print to console/log file.
    * 
    * @return true if the text was written, false if not.
    */
   virtual bool log(
      const char* text, const char* c = NULL, Level level = Error,
      bool header = true, bool useCustomStreams = true);
   
   /**
    * Gets the stack trace from a throwable object.
    * 
    * @param t the throwable object.
    * 
    * @return the stack trace as a string.
    */
   //virtual static const char* getStackTrace(Throwable t);
   
   /**
    * Writes the passed string to this logger for the given class.
    * Level is set to error level.
    *
    * @param text the text to write to the logger.
    * @param c the class to write to the logger for.
    * 
    * @return true if the text was written, false if not.
    */
   virtual bool error(const char* text, const char* c = NULL);
   
   /**
    * Writes the passed string to this logger for the given class.
    * Level is set to warning level.
    *
    * @param text the text to write to this logger.
    * @param c the class to write for.
    * 
    * @return true if the text was written, false if not.
    */
   virtual bool warning(const char* text, const char* c = NULL);

   /**
    * Writes the passed string to this logger for the given class.
    * Level is set to message level.
    *
    * @param text the text to write to this logger.
    * @param c the class to write to this logger for.
    * 
    * @return true if the text was written, false if not.
    */
   virtual bool msg(const char* text, const char* c = NULL);
   
   /**
    * Writes the passed string to this logger for the given class.
    * Level is set to debug level.
    *
    * @param text the text to write to this logger.
    * @param c the class to write to this logger for.
    * 
    * @return true if the text was written, false if not.
    */
   virtual bool debug(const char* text, const char* c = NULL);
   
   /**
    * Writes the passed string to this logger for the given class.
    * Level is set to debug data level.
    *
    * @param text the text to write to this logger.
    * @param c the class to write to this logger for.
    * 
    * @return true if the text was written, false if not.
    */
   virtual bool debugData(const char* text, const char* c = NULL);
   
   /**
    * Writes the passed string to this logger for the given class.
    * Level is set to detail level.
    *
    * @param text the text to write to the log file.
    * @param c the class to write to this logger for.
    * 
    * @return true if the text was written, false if not.
    */
   virtual bool detail(const char* text, const char* c = NULL);
};

} // end namespace logging
} // end namespace db
#endif
