/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_logging_Logging_H
#define db_logging_Logging_H

#include "db/util/Macros.h"
#include "db/logging/Logger.h"
// Include default categories.
#include "db/logging/LoggingCategories.h"

namespace db
{
namespace logging
{

/**
 * Macros to ease logging.
 *  
 * @author David I. Lehn
 */

/**
 * Remove location info when not in debug mode.
 */
#ifndef DB_NDEBUG
#define DB_LOG_STRLOC DB_STRLOC
#else
#define DB_LOG_STRLOC NULL
#endif

/**
 * Root logging macro.
 */
#define DB_LOG(cat, level, object, flags, message) \
   DB_STMT_START { \
      db::logging::Logger::logToLoggers( \
         cat, level, DB_LOG_STRLOC, object, flags, message); \
   } DB_STMT_END

/**
 * Log with a valid object (may be NULL).
 */
#define DB_CAT_LEVEL_OBJECT_LOG(cat, level, object, message) \
   DB_LOG(cat, level, object, db::logging::Logger::LogObjectValid, message)
/**
 * Log with no object.
 */ 
#define DB_CAT_LEVEL_LOG(cat, level, message) \
   DB_LOG(cat, level, NULL, 0, message)

/**
 * Log an error message.
 */
#define DB_CAT_OBJECT_ERROR(cat, object, message) \
   DB_CAT_LEVEL_OBJECT_LOG(cat, db::logging::Logger::Error, object, message)
#define DB_CAT_ERROR(cat, message) \
   DB_CAT_LEVEL_LOG(cat, db::logging::Logger::Error, message)
#define DB_ERROR(message) \
   DB_CAT_ERROR(DB_DEFAULT_CAT, message)

/**
 * Log a warning message.
 */
#define DB_CAT_OBJECT_WARNING(cat, object, message) \
   DB_CAT_LEVEL_OBJECT_LOG(cat, db::logging::Logger::Warning, object, message)
#define DB_CAT_WARNING(cat, message) \
   DB_CAT_LEVEL_LOG(cat, db::logging::Logger::Warning, message)
#define DB_WARNING(message) \
   DB_CAT_WARNING(DB_DEFAULT_CAT, message)

/**
 * Log an info message.
 */
#define DB_CAT_OBJECT_INFO(cat, object, message) \
   DB_CAT_LEVEL_OBJECT_LOG(cat, db::logging::Logger::Info, object, message)
#define DB_CAT_INFO(cat, message) \
   DB_CAT_LEVEL_LOG(cat, db::logging::Logger::Info, message)
#define DB_INFO(message) \
   DB_CAT_INFO(DB_DEFAULT_CAT, message)

/**
 * Log a debug message.  May be compiled out.
 */
#ifndef DB_NDEBUG
#define DB_CAT_OBJECT_DEBUG(cat, object, message) \
   DB_CAT_LEVEL_OBJECT_LOG(cat, db::logging::Logger::Debug, object, message)
#define DB_CAT_DEBUG(cat, message) \
   DB_CAT_LEVEL_LOG(cat, db::logging::Logger::Debug, message)
#define DB_DEBUG(message) \
   DB_CAT_DEBUG(DB_DEFAULT_CAT, message)
#else
#define DB_CAT_OBJECT_DEBUG(cat, object, message) DB_STMT_EMPTY
#define DB_CAT_DEBUG(cat, message)  DB_STMT_EMPTY
#define DB_DEBUG(message)  DB_STMT_EMPTY
#endif

} // end namespace logging
} // end namespace db
#endif
