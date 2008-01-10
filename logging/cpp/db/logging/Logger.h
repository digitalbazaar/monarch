/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_logging_Logger_H
#define db_logging_Logger_H

#include <map>
#include <utility>
#include <list>

#include "db/rt/Object.h"
#include "db/util/Macros.h"
#include "db/logging/Category.h"
#include "db/logging/LoggingCategories.h"

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
   typedef enum LogLevel {
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
    * The logging fields.
    */
   typedef enum LogField {
      /**
       * Date field.
       */
      DateField = 1,
      NameField = (1 << 1),
      LevelField = (1 << 2),
      CategoryField = (1 << 3),
      LocationField = (1 << 4),
      ObjectField = (1 << 5),
      AllFields = (DateField | NameField | LevelField | CategoryField |
         LocationField | ObjectField)
   };
   
   /**
    * The object type.
    */
   typedef enum ObjectType {
      /**
       * Object pointer is invalid.  Log nothing.
       */
      NoObject = 0,
      /**
       * A generic pointer.  Log address.
       */
      Pointer,
      /**
       * A DynamicObject.  Log address and contents.
       */
      DynamicObject
   };
   
   /**
    * Return a string representation of a level.
    * 
    * @param level the Level.
    * 
    * @return string representation of the level.
    */
   static const char* levelToString(LogLevel level);
   
protected:
   
   /**
    * The name of the logger, which is used in debug messages to identify the
    * source of the message.
    */
   const char* mName;
      
   /**
    * The current level setting.
    */
   LogLevel mLevel;
   
   /**
    * The date format.
    */
   char* mDateFormat;

   /**
    * Type to store log fields mask.
    */
   typedef unsigned int LogFields;

   /**
    * Log fields mask.
    */
   LogFields mFields;

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
    * @param name the name of the logger, used in the debug message to identify 
    *             which logger generated the message.
    * @param level the max level to display.
    */
   Logger(const char* name, LogLevel level = None);
   
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
   virtual void setLevel(LogLevel level);

   /**
    * Gets the level set for this logger.
    *
    * @return the level set for this logger.
    */
   virtual LogLevel getLevel();
   
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
   virtual bool setDateFormat(const char* dateFormat);
   
   /**
    * Sets the log fields. If the date format given is not
    * a valid format, the method does nothing but return false.
    *
    * @param fields a bit mask of fields to print out.
    */
   virtual void setLogFields(unsigned int fields);
   
   /**
    * Log a message.  The implementation of this method should lock the logger,
    * check the the log level, create a formatted message, and call the simple
    * log(message) method as needed to perform message output.
    *
    * @param cat the message category name or NULL
    * @param level the message level
    * @param location the location of this log call (or NULL) (see DB_STRLOC)
    * @param objectType the type of the object pointer
    * @param object the object being debugged (or NULL)
    * @param message the log message
    * @param header true to use the logger's header, false not to.
    * 
    * @return true if the text was written, false if not.
    */
   bool log(
      db::logging::Category* cat, 
      LogLevel level,
      const char* location,
      ObjectType objectType,
      const void* object,
      const char* message);
   
   /**
    * Log a message to all loggers registered for this category.
    *
    * @param cat the message category id
    * @param level the message level
    * @param location the location of this log call (or NULL) (see DB_STRLOC)
    * @param objectType the type of the object pointer
    * @param object the object being debugged (or NULL)
    * @param message the log message
    * @param header true to use the logger's header, false not to.
    */
   static void logToLoggers(
      db::logging::Category* cat,
      LogLevel level,
      const char* location,
      ObjectType objectType,
      const void* object,
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
      db::logging::Category* category = DB_DEFAULT_CAT);

   /**
    * Remove a logger for a category.  This will remove the first match if
    * duplicates are found.
    * 
    * @param logger the logger to register
    * @param category the category to use.  Defaults to a generic category.
    */
   static void removeLogger(Logger* logger,
      db::logging::Category* category = DB_DEFAULT_CAT);

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

#define DB_LOG(cat, level, type, object, message) \
   DB_STMT_START { \
   db::logging::Logger::logToLoggers( \
      cat, level, DB_STRLOC, \
      type, object, \
      message); \
   } DB_STMT_END

#define DB_CAT_LEVEL_OBJECT_LOG(cat, level, object, message) \
   DB_LOG(cat, level, db::logging::Logger::Pointer, object, message)
#define DB_CAT_LEVEL_DYNO_LOG(cat, level, dyno, message) \
   DB_LOG(cat, level, db::logging::Logger::DynamicObject, dyno, message)

#define DB_CAT_OBJECT_ERROR(cat, object, message) \
   DB_CAT_LEVEL_OBJECT_LOG(cat, db::logging::Logger::Error, object, message)
#define DB_CAT_DYNO_ERROR(cat, dyno, message) \
   DB_CAT_LEVEL_DYNO_LOG(cat, db::logging::Logger::Error, dyno, message)
#define DB_CAT_ERROR(cat, message) \
   DB_CAT_OBJECT_ERROR(cat, NULL, message)
#define DB_ERROR(message) \
   DB_CAT_ERROR(DB_DEFAULT_CAT, message)
#define DB_DYNO_ERROR(dyno, message) \
   DB_CAT_DYNO_ERROR(DB_DEFAULT_CAT, dyno, message)

#define DB_CAT_OBJECT_WARNING(cat, object, message) \
   DB_CAT_LEVEL_OBJECT_LOG(cat, db::logging::Logger::Warning, object, message)
#define DB_CAT_DYNO_WARNING(cat, dyno, message) \
   DB_CAT_LEVEL_DYNO_LOG(cat, db::logging::Logger::Warning, dyno, message)
#define DB_CAT_WARNING(cat, message) \
   DB_CAT_OBJECT_WARNING(cat, NULL, message)
#define DB_WARNING(message) \
   DB_CAT_WARNING(DB_DEFAULT_CAT, message)
#define DB_DYNO_WARNING(dyno, message) \
   DB_CAT_DYNO_WARNING(DB_DEFAULT_CAT, dyno, message)

#define DB_CAT_OBJECT_INFO(cat, object, message) \
   DB_CAT_LEVEL_OBJECT_LOG(cat, db::logging::Logger::Info, object, message)
#define DB_CAT_DYNO_INFO(cat, dyno, message) \
   DB_CAT_LEVEL_DYNO_LOG(cat, db::logging::Logger::Info, dyno, message)
#define DB_CAT_INFO(cat, message) \
   DB_CAT_OBJECT_INFO(cat, NULL, message)
#define DB_INFO(message) \
   DB_CAT_INFO(DB_DEFAULT_CAT, message)
#define DB_DYNO_INFO(dyno, message) \
   DB_CAT_DYNO_INFO(DB_DEFAULT_CAT, dyno, message)

#define DB_CAT_OBJECT_DEBUG(cat, object, message) \
   DB_CAT_LEVEL_OBJECT_LOG(cat, db::logging::Logger::Debug, object, message)
#define DB_CAT_DYNO_DEBUG(cat, dyno, message) \
   DB_CAT_LEVEL_DYNO_LOG(cat, db::logging::Logger::Debug, dyno, message)
#define DB_CAT_DEBUG(cat, message) \
   DB_CAT_OBJECT_DEBUG(cat, NULL, message)
#define DB_DEBUG(message) \
   DB_CAT_DEBUG(DB_DEFAULT_CAT, message)
#define DB_DYNO_DEBUG(dyno, message) \
   DB_CAT_DYNO_DEBUG(DB_DEFAULT_CAT, dyno, message)

} // end namespace logging
} // end namespace db
#endif
