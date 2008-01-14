/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_logging_Logger_H
#define db_logging_Logger_H

#include <map>
#include <utility>
#include <list>

#include "db/rt/Object.h"
#include "db/logging/Category.h"

namespace db
{
namespace logging
{

/**
 * A class that handles logging to a file.
 *  
 * @author Dave Longley
 * @author David I. Lehn
 * @author Manu Sporny
 */
class Logger : public virtual db::rt::Object
{
public:
   /**
    * The logging level.
    */
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
       * The info level setting. Errors, warnings, and info will be
       * logged.
       */
      Info,
      /**
       * The debug level setting. Errors, warnings, info, and debug
       * information will be logged.
       * 
       * Debug log output should include stack traces, etc.
       */
      Debug,
      /**
       * The debug data level setting. Errors, warnings, info, debug,
       * and debug data will be logged.
       * 
       * Debug data log output includes any data associated with debug log output.
       */
      DebugData,
      /**
       * The detail level setting. Errors, warnings, info, debug,
       * debug data, and debug fine detail will be logged.
       * 
       * Detail log output includes very fine detailed informational messages.
       */
      DebugDetail,
      /**
       * The maximum level setting. Everything will be logged.
       */
      Max
   };
   
   /**
    * Logger control flags.
    */
   typedef enum LoggerFlag {
      /**
       * Log date.
       */
      LogDate = 1,
      /**
       * Log thread name.
       */
      LogThread = (1 << 1),
      /**
       * Log object address.
       */
      LogObject = (1 << 2),
      /**
       * Log message level.
       */
      LogLevel = (1 << 3),
      /**
       * Log message category.
       */
      LogCategory = (1 << 4),
      /**
       * Log message origin location.
       */
      LogLocation = (1 << 5),
      /**
       * Default logging flag (Log date, object, level, and category).
       */
      LogDefaultFlags =
         LogDate | LogObject | LogLevel | LogCategory,
      /**
       * Verbose logging flags (LogDefaultFlags plus thread and location).
       */
      LogVerboseFlags = LogDefaultFlags | LogThread | LogLocation
   };
   
   /**
    * Logger control flags.
    */
   typedef unsigned int LoggerFlags;
   
   /**
    * Log message flags.
    */
   typedef enum LogFlag {
      /**
       * Log object parameter is set. 
       */
      LogObjectValid = 1
   };
   
   /**
    * Log message flags.
    */
   typedef unsigned int LogFlags;
   
   /**
    * Return a string representation of a level.
    * 
    * @param level the Level.
    * 
    * @return string representation of the level.
    */
   static const char* levelToString(Level level);
   
protected:
   /**
    * The current level setting.
    */
   Level mLevel;
   
   /**
    * The date format.
    */
   char* mDateFormat;

   /**
    * Logger flags.
    */
   LoggerFlags mFlags;

   /**
    * A multimap of log categories to many loggers.
    */
   typedef std::multimap<Category*, Logger*> LoggerMap;

   /**
    * Map from categories to loggers.
    */
   static LoggerMap* sLoggers;
   
public:
   /**
    * Creates a new logger with specified level.
    *
    * @param level the max level to display.
    * @param flags a bit field of LoggerFlags.  Default to all.
    */
   Logger(Level level = Max, LoggerFlags flags = LogDefaultFlags);
   
   /**
    * Destructs the Logger.
    */
   virtual ~Logger();
   
   /**
    * Initializes the logger framework. This static method MUST be called
    * during application start-up before any threads are active in order to
    * use the logging framework.
    */
   static void initialize();

   /**
    * Frees all memory used by the logger framework. This static method MUST
    * be called during application tear-down, after all threads have been
    * terminated.
    */
   static void cleanup();
   
   /**
    * Sets the level for this logger.
    *
    * @param level the level to set.
    */
   virtual void setLevel(Level level);

   /**
    * Gets the level set for this logger.
    *
    * @return the level set for this logger.
    */
   virtual Level getLevel();
   
   /**
    * Gets the current date in the appropriate format.
    * 
    * @param date the date string to populate with the current date in
    *             the appropriate format.
    */
   virtual void getDate(std::string& date);
   
   /**
    * Sets the date format. If the date format given is not
    * a valid format, the method does nothing but return false.
    *
    * @param dateFormat the new date format.
    * 
    * @return true if the date format is set, false if not.
    */
   virtual bool setDateFormat(const char* format);
   
   /**
    * Sets the logger flags.
    *
    * @param flags a bit field of LoggerFlags.
    */
   virtual void setFlags(LoggerFlags flags);
   
   /**
    * Gets the logger flags.
    *
    * @return the logger flags.
    */
   virtual LoggerFlags getFlags();
   
   /**
    * Log a message.  The implementation of this method should lock the logger,
    * check the the log level, create a formatted message, and call the simple
    * log(message) method as needed to perform message output.
    *
    * @param cat the message category name or NULL
    * @param level the message level
    * @param location the location of this log call or NULL (see DB_STRLOC)
    * @param object a source object or NULL
    * @param flags flags for this message
    * @param message the log message
    * 
    * @return true if the text was written, false if not.
    */
   bool log(
      db::logging::Category* cat, 
      Level level,
      const char* location,
      const void* object,
      LogFlags flags,
      const char* message);
   
   /**
    * Log a message to all loggers registered for a category.
    *
    * @param registeredCat send to loggers registered with this category
    * @param messageCat the message category
    * @param level the message level
    * @param location the location of this log call (or NULL) (see DB_STRLOC)
    * @param object a source object or NULL
    * @param flags flags for this message
    * @param message the log message
    */
   static void logToLoggers(
      db::logging::Category* registeredCat,
      db::logging::Category* messageCat,
      Level level,
      const char* location,
      const void* object,
      LogFlags flags,
      const char* message);
   
   /**
    * Log a message to all loggers registered for a category and to loggers
    * registered for all categories.
    *
    * @param cat the message category
    * @param level the message level
    * @param location the location of this log call (or NULL) (see DB_STRLOC)
    * @param object a source object or NULL
    * @param flags flags for this message
    * @param message the log message
    */
   static void logToLoggers(
      db::logging::Category* cat,
      Level level,
      const char* location,
      const void* object,
      LogFlags flags,
      const char* message);
   
   /**
    * Logs a pre-formatted message from the default full log() method.
    *
    * @param message the log message.
    */
   virtual void log(const char* message) = 0;
   
   /**
    * Add a logger for a category.  Any number of loggers can be added for a
    * single category.
    * 
    * @param logger the logger to register
    * @param category the category to use.  Defaults to the default category.
    */
   static void addLogger(Logger* logger,
      db::logging::Category* category = DB_ALL_CAT);

   /**
    * Remove a logger for a category.  This will remove the first match if
    * duplicates are found.
    * 
    * @param logger the logger to register
    * @param category the category to use.  Defaults to a generic category.
    */
   static void removeLogger(Logger* logger,
      db::logging::Category* category = DB_ALL_CAT);

   /**
    * Clear all loggers.
    */
   static void clearLoggers();

   /**
    * Gets the loggers.
    * 
    * @return the loggers
    */
   //getLoggers(...)
};

} // end namespace logging
} // end namespace db
#endif
