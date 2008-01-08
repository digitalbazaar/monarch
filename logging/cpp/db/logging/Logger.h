/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_logging_Logger_H
#define db_logging_Logger_H

#include <map>

#include "db/rt/Object.h"
#include "db/io/File.h"
#include "db/io/OutputStream.h"

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
    * The object type.
    */
   typedef enum ObjectType {
      /**
       * A generic pointer.  Log address.
       */
      Pointer = 0,
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
   static const char* levelToString(Level level);
   
   /**
    * The default category.
    */
   static const char* defaultCategory;
   
protected:
   /**
    * A NameComparator compares two logger names.
    */
   typedef struct NameComparator
   {
      /**
       * Compares two null-terminated strings, returning true if the first is
       * less than the second, false if not.
       * 
       * @param s1 the first string.
       * @param s2 the second string.
       * 
       * @return true if the s1 < s2, false if not.
       */
      bool operator()(const char* s1, const char* s2) const
      {
         return strcmp(s1, s2) < 0;
      }
   };
   
   /**
    * multimap from categories to loggers.
    */
   static std::multimap<const char*, Logger*, NameComparator> sLoggers;
   
   /**
    * The name of this logger.
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
   
public:
   /**
    * Creates a new logger with specified level.
    *
    * @param name the name of the logger.
    * @param level the max level to display.
    */
   Logger(const char* name, Level level = None);
   
   /**
    * Destructs the Logger.
    */
   virtual ~Logger();
   
   /**
    * Gets the name of this logger.
    * 
    * @return the name of this logger.
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
   virtual bool setDateFormat(const char* dateFormat);
   
   /**
    * Log a message.  The implementation of this method should lock the logger,
    * check the the log level, create a formatted message, and call the simple
    * log(message) method as needed to perform message output.
    *
    * @param cat the message category (or NULL)
    * @param level the message level
    * @param file the location of this log call (or NULL)
    * @param function the function of this log call (or NULL)
    * @param line the line of this log call (or -1)
    * @param objectType the type of the object pointer
    * @param object the object being debugged (or NULL)
    * @param message the log message
    * @param header true to use the logger's header, false not to.
    * 
    * @return true if the text was written, false if not.
    */
   bool log(
      const char* cat,
      Level level,
      const char* file,
      const char* function,
      int line,
      ObjectType objectType,
      const void* object,
      const char* message);
   
   /**
    * Log a message to all loggers registered for this category.
    *
    * @param cat the message category (or NULL)
    * @param level the message level
    * @param file the location of this log call (or NULL)
    * @param function the function of this log call (or NULL)
    * @param line the line of this log call (or -1)
    * @param objectType the type of the object pointer
    * @param object the object being debugged (or NULL)
    * @param message the log message
    * @param header true to use the logger's header, false not to.
    */
   static void fullLog(
      const char* cat,
      Level level,
      const char* file,
      const char* function,
      int line,
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
    * Gets the stack trace from a throwable object.
    * 
    * @param t the throwable object.
    * 
    * @return the stack trace as a string.
    */
   //static const char* getStackTrace(Throwable t);
   //

   static void addLogger(Logger* logger, const char* category = db::logging::Logger::defaultCategory);
   static void removeLogger(Logger* logger, const char* category = db::logging::Logger::defaultCategory);
   //getLoggers(...)
};

#define DB_STMT_BEGIN do {
#define DB_STMT_END } while(0);

#define DB_FULL_LOG(cat, level, type, object, message) \
   DB_STMT_BEGIN \
   db::logging::Logger::fullLog( \
      cat, level, __FILE__, __func__, __LINE__, \
      type, object, \
      message); \
   DB_STMT_END

#define DB_CAT_LEVEL_LOG(cat, level, object, message) \
   DB_FULL_LOG(cat, level, db::logging::Logger::Pointer, object, message)

#define DB_CAT_OBJECT_ERROR(cat, object, message) \
   DB_CAT_LEVEL_LOG(cat, db::logging::Logger::Error, object, message)
#define DB_CAT_ERROR(cat, message) \
   DB_CAT_OBJECT_ERROR(cat, NULL, message)
#define DB_ERROR(message) \
   DB_CAT_ERROR(db::logging::Logger::defaultCategory, message)

#define DB_CAT_OBJECT_WARNING(cat, object, message) \
   DB_CAT_LEVEL_LOG(cat, db::logging::Logger::Warning, object, message)
#define DB_CAT_WARNING(cat, message) \
   DB_CAT_OBJECT_WARNING(cat, NULL, message)
#define DB_WARNING(message) \
   DB_CAT_WARNING(db::logging::Logger::defaultCategory, message)

#define DB_CAT_OBJECT_INFO(cat, object, message) \
   DB_CAT_LEVEL_LOG(cat, db::logging::Logger::Info, object, message)
#define DB_CAT_INFO(cat, message) \
   DB_CAT_OBJECT_INFO(cat, NULL, message)
#define DB_INFO(message) \
   DB_CAT_INFO(db::logging::Logger::defaultCategory, message)

#define DB_CAT_OBJECT_DEBUG(cat, object, message) \
   DB_CAT_LEVEL_LOG(cat, db::logging::Logger::Debug, object, message)
#define DB_CAT_DEBUG(cat, message) \
   DB_CAT_OBJECT_DEBUG(cat, NULL, message)
#define DB_DEBUG(message) \
   DB_CAT_DEBUG(db::logging::Logger::defaultCategory, message)

/**
 * Special basic support for debugging DynamicObjects
 */
#define DB_DEBUG_DYNO(dyno, message) \
   DB_FULL_LOG(db::logging::Logger::defaultCategory, \
      db::logging::Logger::Debug, \
      db::logging::Logger::DynamicObject, \
      dyno, message)

} // end namespace logging
} // end namespace db
#endif
