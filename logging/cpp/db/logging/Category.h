/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_logging_Category_H
#define db_logging_Category_H

#include <map>
#include <utility>
#include <list>

#include "db/rt/Object.h"
#include "db/util/Macros.h"

namespace db
{
   namespace logging
   {
      class Category;
   }
}
extern db::logging::Category* DB_DEFAULT_CAT;
extern db::logging::Category* DB_ALL_CAT;

namespace db
{
namespace logging
{

/**
 * A logging category.
 *  
 * @author David I. Lehn
 */
class Category
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
    * The name of the logger.  Used in log messages to identify the source of
    * the message.
    */
   char* mName;
   
   /**
    * The short name of the logger.  Used in log messages to identify the
    * source of the message.  If set, this may be used by loggers to simplify
    * output.
    */
   char* mShortName;
   
   /**
    * A description of this category.  Used for help messages.
    */
   char* mDescription;
   
public:
   /**
    * Create a new Category.
    *
    * @param name the name of the logger.  Used in log messages to identify the
    *        source of the message.
    * @param shortName the short name of the logger.  Used in log messages to
    *        identify the source of the message.  If set, this may be used by
    *        loggers to simplify output.  May be NULL. 
    * @param description a description of this category.  Used for help
    *        messages.  May be NULL.
    */
   Category(const char* name, const char* shortName, const char* description);
   
   /**
    * Destructs the Category.
    */
   virtual ~Category();
   
   /**
    * Initializes the static categories. This static method is called by
    * Logging::initialize() which MUST be called during application start-up.
    */
   static void initialize();

   /**
    * Frees static categories. This static method is called from
    * Logging::cleanup() and MUST be called during application tear-down.
    */
   static void cleanup();
   
   /**
    * Set the category name.
    *
    * @param name the category name.
    */
   virtual void setName(const char* name);

   /**
    * Gets the category name.
    *
    * @return the category name or "<?>" if not set.
    */
   virtual const char* getName();
   
   /**
    * Set the category short name.
    *
    * @param name the category short name.
    */
   virtual void setShortName(const char* shortName);

   /**
    * Gets the category short name.
    *
    * @return the category short name or NULL if not set.
    */
   virtual const char* getShortName();
   
   /**
    * Set the category description.
    *
    * @param name the category description.
    */
   virtual void setDescription(const char* description);

   /**
    * Gets the category description.
    *
    * @return the category description or NULL if not set.
    */
   virtual const char* getDescription();
};

} // end namespace logging
} // end namespace db
#endif
