/*
 * Copyright (c) 2008-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_logging_Logging_H
#define monarch_logging_Logging_H

#include "monarch/util/Macros.h"
#include "monarch/logging/Category.h"
#include "monarch/logging/Logger.h"
// Include standard categories for convienience:
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
#if !defined(MO_DISABLE_LOG_LINE_NUMBERS) && !defined(MO_NDEBUG)
#define MO_LOG_STRLOC MO_STRLOC
#else
#define MO_LOG_STRLOC NULL
#endif

/* FIXME: implement these vararg compiler type checks, add to a installed
 *        config file and implement everything below.  Just using GNUC for now.
 */
/*
#ifdef MO_HAVE_ISO_VARARGS
#define MO_X(a, b, loc, ...) MO_Y(a, b, loc, __VA_ARGS__)
...
#elif defined(MO_HAVE_GNUC_VARARGS)
#define MO_X(a, b, loc, args...) MO_Y(a, b, loc, ##args)
#else // no macro vararg support, inline
static inline void MO_X(monarch::a a, monarch::b b, const char* loc,
   const char* format, ...)
{
   va_list varargs;
   va_start(varargs, format);
   MO_Y(a, b, NULL, format, varargs);
   va_end(varargs);
}
#endif
*/

/**
 * Root logging macro.
 */
#define MO_LOG(cat, level, object, flags, args...) \
   MO_STMT_START { \
      monarch::logging::Logger::logToLoggers( \
         cat, level, MO_LOG_STRLOC, object, flags, ##args); \
   } MO_STMT_END

/**
 * Log with a valid object (may be NULL).
 */
#define MO_CAT_LEVEL_OBJECT_LOG(cat, level, object, args...) \
   MO_LOG(cat, level, object, monarch::logging::Logger::LogObjectValid, ##args)
/**
 * Log with no object.
 */
#define MO_CAT_LEVEL_LOG(cat, level, args...) \
   MO_LOG(cat, level, NULL, 0, ##args)

/**
 * Log an error message.
 */
#define MO_CAT_OBJECT_ERROR(cat, object, args...) \
   MO_CAT_LEVEL_OBJECT_LOG(cat, monarch::logging::Logger::Error, object, ##args)
#define MO_CAT_ERROR(cat, args...) \
   MO_CAT_LEVEL_LOG(cat, monarch::logging::Logger::Error, ##args)
#define MO_ERROR(args...) \
   MO_CAT_ERROR(MO_DEFAULT_CAT, ##args)

/**
 * Log a warning message.
 */
#define MO_CAT_OBJECT_WARNING(cat, object, args...) \
   MO_CAT_LEVEL_OBJECT_LOG( \
      cat, monarch::logging::Logger::Warning, object, ##args)
#define MO_CAT_WARNING(cat, args...) \
   MO_CAT_LEVEL_LOG(cat, monarch::logging::Logger::Warning, ##args)
#define MO_WARNING(args...) \
   MO_CAT_WARNING(MO_DEFAULT_CAT, ##args)

/**
 * Log an info message.
 */
#define MO_CAT_OBJECT_INFO(cat, object, args...) \
   MO_CAT_LEVEL_OBJECT_LOG(cat, monarch::logging::Logger::Info, object, ##args)
#define MO_CAT_INFO(cat, args...) \
   MO_CAT_LEVEL_LOG(cat, monarch::logging::Logger::Info, ##args)
#define MO_INFO(args...) \
   MO_CAT_INFO(MO_DEFAULT_CAT, ##args)

/**
 * Log a debug message.  May be compiled out.
 */
#ifndef MO_NDEBUG
#define MO_CAT_OBJECT_DEBUG(cat, object, args...) \
   MO_CAT_LEVEL_OBJECT_LOG(cat, monarch::logging::Logger::Debug, object, ##args)
#define MO_CAT_DEBUG(cat, args...) \
   MO_CAT_LEVEL_LOG(cat, monarch::logging::Logger::Debug, ##args)
#define MO_DEBUG(args...) \
   MO_CAT_DEBUG(MO_DEFAULT_CAT, ##args)
#else
#define MO_CAT_OBJECT_DEBUG(cat, object, args...) MO_STMT_EMPTY
#define MO_CAT_DEBUG(cat, args...) MO_STMT_EMPTY
#define MO_OBJ_DEBUG(args...) MO_STMT_EMPTY
#define MO_DEBUG(args...) MO_STMT_EMPTY
#endif

/**
 * Log a debug-data message.  May be compiled out.
 */
#ifndef MO_NDEBUG
#define MO_CAT_OBJECT_DEBUG_DATA(cat, object, args...) \
   MO_CAT_LEVEL_OBJECT_LOG( \
      cat, monarch::logging::Logger::DebugData, object, ##args)
#define MO_CAT_DEBUG_DATA(cat, args...) \
   MO_CAT_LEVEL_LOG(cat, monarch::logging::Logger::DebugData, ##args)
#define MO_DEBUG_DATA(args...) \
   MO_CAT_DEBUG_DATA(MO_DEFAULT_CAT, ##args)
#else
#define MO_CAT_OBJECT_DEBUG_DATA(cat, object, args...) MO_STMT_EMPTY
#define MO_CAT_DEBUG_DATA(cat, args...) MO_STMT_EMPTY
#define MO_OBJ_DEBUG_DATA(args...) MO_STMT_EMPTY
#define MO_DEBUG_DATA(args...) MO_STMT_EMPTY
#endif

/**
 * Log a debug-detail message.  May be compiled out.
 */
#ifndef MO_NDEBUG
#define MO_CAT_OBJECT_DEBUG_DETAIL(cat, object, args...) \
   MO_CAT_LEVEL_OBJECT_LOG( \
      cat, monarch::logging::Logger::DebugDetail, object, ##args)
#define MO_CAT_DEBUG_DETAIL(cat, args...) \
   MO_CAT_LEVEL_LOG(cat, monarch::logging::Logger::DebugDetail, ##args)
#define MO_DEBUG_DETAIL(args...) \
   MO_CAT_DEBUG_DETAIL(MO_DEFAULT_CAT, ##args)
#else
#define MO_CAT_OBJECT_DEBUG_DETAIL(cat, object, args...) MO_STMT_EMPTY
#define MO_CAT_DEBUG_DETAIL(cat, args...) MO_STMT_EMPTY
#define MO_OBJ_DEBUG_DETAIL(args...) MO_STMT_EMPTY
#define MO_DEBUG_DETAIL(args...) MO_STMT_EMPTY
#endif

} // end namespace logging
} // end namespace monarch
#endif
