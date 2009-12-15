/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_logging_Logging_H
#define monarch_logging_Logging_H

#include "monarch/util/Macros.h"
#include "monarch/logging/Category.h"
#include "monarch/logging/Logger.h"
// Include DB categories for convienience:
#include "monarch/logging/LoggingCategories.h"

namespace monarch
{
namespace logging
{

/**
 * Pseudo-class to initialize and cleanup the logging framework.
 *
 * @author David I. Lehn
 */
class Logging
{
public:
   /**
    * Initializes the category system, static categories, and the static Logger.
    * This MUST be called during application start-up.
    *
    * @return true on success, false on failure and exception set
    */
   static bool initialize();

   /**
    * Cleans up the static Logger, static categories, and category system.
    * This MUST be called during application tear-down.
    */
   static void cleanup();
};


/**
 * Macros to ease logging.
 */

/**
 * Remove location info when disabled or when not in debug mode.
 */
#if !defined(DB_DISABLE_LOG_LINE_NUMBERS) && !defined(DB_NDEBUG)
#define DB_LOG_STRLOC DB_STRLOC
#else
#define DB_LOG_STRLOC NULL
#endif

/* FIXME: implement these vararg compiler type checks, add to a installed
 *        config file and implement everything below.  Just using GNUC for now.
 */
/*
#ifdef DB_HAVE_ISO_VARARGS
#define DB_X(a, b, loc, ...) DB_Y(a, b, loc, __VA_ARGS__)
...
#elif defined(DB_HAVE_GNUC_VARARGS)
#define DB_X(a, b, loc, args...) DB_Y(a, b, loc, ##args)
#else // no macro vararg support, inline
static inline void DB_X(monarch::a a, monarch::b b, const char* loc,
   const char* format, ...)
{
   va_list varargs;
   va_start(varargs, format);
   DB_Y(a, b, NULL, format, varargs);
   va_end(varargs);
}
#endif
*/

/**
 * Root logging macro.
 */
#define DB_LOG(cat, level, object, flags, args...) \
   DB_STMT_START { \
      monarch::logging::Logger::logToLoggers( \
         cat, level, DB_LOG_STRLOC, object, flags, ##args); \
   } DB_STMT_END

/**
 * Log with a valid object (may be NULL).
 */
#define DB_CAT_LEVEL_OBJECT_LOG(cat, level, object, args...) \
   DB_LOG(cat, level, object, monarch::logging::Logger::LogObjectValid, ##args)
/**
 * Log with no object.
 */
#define DB_CAT_LEVEL_LOG(cat, level, args...) \
   DB_LOG(cat, level, NULL, 0, ##args)

/**
 * Log an error message.
 */
#define DB_CAT_OBJECT_ERROR(cat, object, args...) \
   DB_CAT_LEVEL_OBJECT_LOG(cat, monarch::logging::Logger::Error, object, ##args)
#define DB_CAT_ERROR(cat, args...) \
   DB_CAT_LEVEL_LOG(cat, monarch::logging::Logger::Error, ##args)
#define DB_ERROR(args...) \
   DB_CAT_ERROR(DB_DEFAULT_CAT, ##args)

/**
 * Log a warning message.
 */
#define DB_CAT_OBJECT_WARNING(cat, object, args...) \
   DB_CAT_LEVEL_OBJECT_LOG( \
      cat, monarch::logging::Logger::Warning, object, ##args)
#define DB_CAT_WARNING(cat, args...) \
   DB_CAT_LEVEL_LOG(cat, monarch::logging::Logger::Warning, ##args)
#define DB_WARNING(args...) \
   DB_CAT_WARNING(DB_DEFAULT_CAT, ##args)

/**
 * Log an info message.
 */
#define DB_CAT_OBJECT_INFO(cat, object, args...) \
   DB_CAT_LEVEL_OBJECT_LOG(cat, monarch::logging::Logger::Info, object, ##args)
#define DB_CAT_INFO(cat, args...) \
   DB_CAT_LEVEL_LOG(cat, monarch::logging::Logger::Info, ##args)
#define DB_INFO(args...) \
   DB_CAT_INFO(DB_DEFAULT_CAT, ##args)

/**
 * Log a debug message.  May be compiled out.
 */
#ifndef DB_NDEBUG
#define DB_CAT_OBJECT_DEBUG(cat, object, args...) \
   DB_CAT_LEVEL_OBJECT_LOG(cat, monarch::logging::Logger::Debug, object, ##args)
#define DB_CAT_DEBUG(cat, args...) \
   DB_CAT_LEVEL_LOG(cat, monarch::logging::Logger::Debug, ##args)
#define DB_DEBUG(args...) \
   DB_CAT_DEBUG(DB_DEFAULT_CAT, ##args)
#else
#define DB_CAT_OBJECT_DEBUG(cat, object, args...) DB_STMT_EMPTY
#define DB_CAT_DEBUG(cat, args...) DB_STMT_EMPTY
#define DB_OBJ_DEBUG(args...) DB_STMT_EMPTY
#define DB_DEBUG(args...) DB_STMT_EMPTY
#endif

/**
 * Log a debug-data message.  May be compiled out.
 */
#ifndef DB_NDEBUG
#define DB_CAT_OBJECT_DEBUG_DATA(cat, object, args...) \
   DB_CAT_LEVEL_OBJECT_LOG( \
      cat, monarch::logging::Logger::DebugData, object, ##args)
#define DB_CAT_DEBUG_DATA(cat, args...) \
   DB_CAT_LEVEL_LOG(cat, monarch::logging::Logger::DebugData, ##args)
#define DB_DEBUG_DATA(args...) \
   DB_CAT_DEBUG_DATA(DB_DEFAULT_CAT, ##args)
#else
#define DB_CAT_OBJECT_DEBUG_DATA(cat, object, args...) DB_STMT_EMPTY
#define DB_CAT_DEBUG_DATA(cat, args...) DB_STMT_EMPTY
#define DB_OBJ_DEBUG_DATA(args...) DB_STMT_EMPTY
#define DB_DEBUG_DATA(args...) DB_STMT_EMPTY
#endif

/**
 * Log a debug-detail message.  May be compiled out.
 */
#ifndef DB_NDEBUG
#define DB_CAT_OBJECT_DEBUG_DETAIL(cat, object, args...) \
   DB_CAT_LEVEL_OBJECT_LOG( \
      cat, monarch::logging::Logger::DebugDetail, object, ##args)
#define DB_CAT_DEBUG_DETAIL(cat, args...) \
   DB_CAT_LEVEL_LOG(cat, monarch::logging::Logger::DebugDetail, ##args)
#define DB_DEBUG_DETAIL(args...) \
   DB_CAT_DEBUG_DETAIL(DB_DEFAULT_CAT, ##args)
#else
#define DB_CAT_OBJECT_DEBUG_DETAIL(cat, object, args...) DB_STMT_EMPTY
#define DB_CAT_DEBUG_DETAIL(cat, args...) DB_STMT_EMPTY
#define DB_OBJ_DEBUG_DETAIL(args...) DB_STMT_EMPTY
#define DB_DEBUG_DETAIL(args...) DB_STMT_EMPTY
#endif

} // end namespace logging
} // end namespace monarch
#endif
