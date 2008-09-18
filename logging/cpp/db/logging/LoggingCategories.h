/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_logging_LoggingCategories_H
#define db_logging_LoggingCategories_H

#include "db/logging/Category.h"
#include "db/rt/WindowsSupport.h"

#ifdef WIN32
#   ifdef BUILD_DB_LOGGING_DLL
#      define DLL_DATA __WIN32_DLL_EXPORT extern
#   else
#      define DLL_DATA __WIN32_DLL_IMPORT
#   endif
#else
#   define DLL_DATA extern
#endif

// This is a list of pre-defined logging categories that can be used by any
// module that would like to log messages to a specific category.

DLL_DATA db::logging::Category* DB_APP_CAT;
DLL_DATA db::logging::Category* DB_CONFIG_CAT;
DLL_DATA db::logging::Category* DB_CRYPTO_CAT; 
DLL_DATA db::logging::Category* DB_DATA_CAT;
DLL_DATA db::logging::Category* DB_EVENT_CAT;
DLL_DATA db::logging::Category* DB_GUI_CAT;
DLL_DATA db::logging::Category* DB_IO_CAT;
DLL_DATA db::logging::Category* DB_LOGGING_CAT;
DLL_DATA db::logging::Category* DB_MAIL_CAT;
DLL_DATA db::logging::Category* DB_MODEST_CAT;
DLL_DATA db::logging::Category* DB_NET_CAT;
DLL_DATA db::logging::Category* DB_RT_CAT;
DLL_DATA db::logging::Category* DB_SPHINX_CAT;
DLL_DATA db::logging::Category* DB_SQL_CAT;
DLL_DATA db::logging::Category* DB_UTIL_CAT;

#undef DLL_DATA

namespace db
{
namespace logging
{

/**
 * Pseudo-class to contain category initialization/cleanup.
 * 
 * FIXME: Move these to modules?
 */
class LoggingCategories
{
public:
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
};

} // end namespace logging
} // end namespace db

#endif
