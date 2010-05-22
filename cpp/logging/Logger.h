/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_logging_Logger_H
#define monarch_logging_Logger_H

#include <cstdarg>
#include <map>
#include <utility>
#include <list>
#include <string>

#include "monarch/rt/SharedLock.h"
#include "monarch/rt/Collectable.h"
#include "monarch/logging/Category.h"

namespace monarch
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
class Logger
{
public:
   /**
    * The logging level.
    */
   enum Level {
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
       * Debug data log output includes any data associated with debug log
       * output.
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
   enum {
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
       * Log messages using any available ANSI color codes.
       */
      LogColor = (1 << 6),
      /**
       * Default logging flag (Log date, object, level, and category).
       */
      LogDefaultFlags =
         LogDate | LogObject | LogLevel | LogCategory,
      /**
       * Verbose logging flags (LogDefaultFlags plus thread and location).
       */
      LogVerboseFlags = LogDefaultFlags | LogThread | LogLocation,
      /**
       * Shift value for last flag. Subclasses can use flag bits greater than
       * (1 << LogLastFlagShift).
       */
      LogLastFlagShift = 6
   };

   /**
    * Logger control flags.
    */
   typedef unsigned int LoggerFlags;

   /**
    * Log message flags.
    */
   enum LogFlag {
      /**
       * Log object parameter is set.
       */
      LogObjectValid = 1
   };

   /**
    * Log message flags.
    */
   typedef unsigned int LogFlags;

protected:
   /**
    * A name for this logger.
    */
   char* mName;

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
    * Internal data structures lock.
    */
   monarch::rt::SharedLock mLock;

   /**
    * A multimap of log categories to many loggers.
    */
   typedef std::multimap<Category*, monarch::rt::Collectable<Logger> >
      LoggerMap;

   /**
    * Map from categories to loggers.
    */
   static LoggerMap* sLoggers;

public:
   /**
    * Creates a new logger with Max level and LogDefaultFlags flags.
    */
   Logger();

   /**
    * Destructs the Logger.
    */
   virtual ~Logger();

   /**
    * Sets the name for this logger.
    *
    * @param name the name to set, NULL to clear the name.
    */
   virtual void setName(const char* name);

   /**
    * Gets the name for this logger.
    *
    * @return the name for this logger, can be NULL.
    */
   virtual const char* getName();

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
    * Sets all the logger flags.
    *
    * @param flags a bit field of all LoggerFlags.
    */
   virtual void setAllFlags(LoggerFlags flags);

   /**
    * Sets the logger flags.
    *
    * @param flags a bit field of LoggerFlags to set.
    */
   virtual void setFlags(LoggerFlags flags);

   /**
    * Sets the logger flags.
    *
    * @param flags a bit field of LoggerFlags to clear.
    */
   virtual void clearFlags(LoggerFlags flags);

   /**
    * Gets the logger flags.
    *
    * @return the logger flags.
    */
   virtual LoggerFlags getFlags();

   /**
    * Log a message. The implementation of this method should lock the logger,
    * check the the log level, create a formatted message, and call the simple
    * log(message) method as needed to perform message output.
    *
    * @param cat the message category name or NULL
    * @param level the message level
    * @param location the location of this log call or NULL (see MO_STRLOC)
    * @param object a source object or NULL
    * @param flags flags for this message
    * @param format the log message format (printf style)
    * @param va_list the log message args
    *
    * @return true if the text was written, false if not.
    */
   virtual bool vLog(
      monarch::logging::Category* cat,
      Level level,
      const char* location,
      const void* object,
      LogFlags flags,
      const char* format,
      va_list varargs);

   /**
    * Log a message. The implementation of this method should lock the logger,
    * check the the log level, create a formatted message, and call the simple
    * log(message) method as needed to perform message output.
    *
    * @param cat the message category name or NULL
    * @param level the message level
    * @param location the location of this log call or NULL (see MO_STRLOC)
    * @param object a source object or NULL
    * @param flags flags for this message
    * @param format the log message format (printf style)
    * @param ... the log message args
    *
    * @return true if the text was written, false if not.
    */
   virtual bool log(
      monarch::logging::Category* cat,
      Level level,
      const char* location,
      const void* object,
      LogFlags flags,
      const char* format,
      ...)
#ifdef __GNUC__
      __attribute__ ((format (printf, 7, 8)))
#endif
         ;

   /**
    * Logs a pre-formatted message from the default full log() method.
    *
    * @param message the log message.
    * @param length length of message
    */
   virtual void log(const char* message, size_t length) = 0;

   /**
    * Explicitly flush any output that hasn't been flushed yet.
    */
   virtual void flush();

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
    * Add a logger for a category. Any number of loggers can be added for a
    * single category.
    *
    * @param logger the logger to register
    * @param category the category to use. Defaults to the default category.
    */
   static void addLogger(
      monarch::rt::Collectable<Logger> logger,
      monarch::logging::Category* category = MO_ALL_CAT);

   /**
    * Remove a logger for a category. This will remove the first match if
    * duplicates are found.
    *
    * @param logger the logger to register
    * @param category the category to use. Defaults to a generic category.
    */
   static void removeLogger(
      monarch::rt::Collectable<Logger>& logger,
      monarch::logging::Category* category = MO_ALL_CAT);

   /**
    * Remove a logger for a category by its name. This will remove the first
    * match if duplicates are found.
    *
    * @param loggerName the name of the logger to remove.
    * @param category the category to use. Defaults to a generic category.
    *
    * @return the removed logger.
    */
   static monarch::rt::Collectable<Logger> removeLoggerByName(
      const char* loggerName,
      monarch::logging::Category* category = MO_ALL_CAT);

   /**
    * Clear all loggers.
    */
   static void clearLoggers();

   /**
    * Flushes all loggers.
    */
   static void flushLoggers();

   /**
    * Gets the loggers.
    *
    * @return the loggers
    */
   //getLoggers(...)

   /**
    * Case insensitive conversion from string to Level.
    *
    * @param slevel the string to convert.
    * @param level the level.
    *
    * @return true if found and level will be set, false if not found.
    */
   static bool stringToLevel(const char *slevel, Level& level);

   /**
    * Conversion from Level to string
    *
    * @param type the Level to convert.
    * @param color true to use ANSI colors, false for normal text.
    *
    * @return the string or NULL.
    */
   static const char* levelToString(Level level, bool color = false);

   /**
    * Log a message to all loggers registered for a category.
    *
    * @param registeredCat send to loggers registered with this category
    * @param messageCat the message category
    * @param level the message level
    * @param location the location of this log call (or NULL) (see MO_STRLOC)
    * @param object a source object or NULL
    * @param flags flags for this message
    * @param format the log message format (printf style)
    * @param va_list the log message args
    */
   static void vLogToLoggers(
      monarch::logging::Category* registeredCat,
      monarch::logging::Category* messageCat,
      Level level,
      const char* location,
      const void* object,
      LogFlags flags,
      const char* format,
      va_list varargs);

   /**
    * Log a message to all loggers registered for a category.
    *
    * @param registeredCat send to loggers registered with this category
    * @param messageCat the message category
    * @param level the message level
    * @param location the location of this log call (or NULL) (see MO_STRLOC)
    * @param object a source object or NULL
    * @param flags flags for this message
    * @param format the log message format (printf style)
    * @param ... the log message args
    */
   static void logToLoggers(
      monarch::logging::Category* registeredCat,
      monarch::logging::Category* messageCat,
      Level level,
      const char* location,
      const void* object,
      LogFlags flags,
      const char* format,
      ...)
#ifdef __GNUC__
      __attribute__ ((format (printf, 7, 8)))
#endif
         ;

   /**
    * Log a message to all loggers registered for a category and to loggers
    * registered for all categories.
    *
    * @param cat the message category
    * @param level the message level
    * @param location the location of this log call (or NULL) (see MO_STRLOC)
    * @param object a source object or NULL
    * @param flags flags for this message
    * @param format the log message format (printf style)
    * @param va_list the log message args
    */
   static void vLogToLoggers(
      monarch::logging::Category* cat,
      Level level,
      const char* location,
      const void* object,
      LogFlags flags,
      const char* format,
      va_list varargs);

   /**
    * Log a message to all loggers registered for a category and to loggers
    * registered for all categories.
    *
    * @param cat the message category
    * @param level the message level
    * @param location the location of this log call (or NULL) (see MO_STRLOC)
    * @param object a source object or NULL
    * @param flags flags for this message
    * @param format the log message format (printf style)
    * @param ... the log message args
    */
   static void logToLoggers(
      monarch::logging::Category* cat,
      Level level,
      const char* location,
      const void* object,
      LogFlags flags,
      const char* format,
      ...)
#ifdef __GNUC__
      __attribute__ ((format (printf, 6, 7)))
#endif
         ;
};

// type definition for a reference counted Logger
typedef monarch::rt::Collectable<Logger> LoggerRef;

} // end namespace logging
} // end namespace monarch
#endif
