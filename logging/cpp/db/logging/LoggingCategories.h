/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_logging_LoggingCategories_H
#define db_logging_LoggingCategories_H

#include "db/logging/Category.h"

// This is a list of pre-defined logging categories that can be used by any
// module that would like to log messages to a specific category.

#ifndef __GNUC__
# define __DLL_IMPORT __declspec(dllimport)
#else
# define __DLL_IMPORT __attribute__((dllimport)) extern
#endif

#if defined (BUILD_LOGGING_DLL) || !defined (__WIN32__)
# define DLL_IMPORT extern
#else
# define DLL_IMPORT __DLL_IMPORT
#endif

DLL_IMPORT db::logging::Category* DB_ALL_CAT;
DLL_IMPORT db::logging::Category* DB_CONFIG_CAT;
DLL_IMPORT db::logging::Category* DB_CRYPTO_CAT; 
DLL_IMPORT db::logging::Category* DB_DATA_CAT;
DLL_IMPORT db::logging::Category* DB_EVENT_CAT;
DLL_IMPORT db::logging::Category* DB_GUI_CAT;
DLL_IMPORT db::logging::Category* DB_IO_CAT;
DLL_IMPORT db::logging::Category* DB_LOGGING_CAT;
DLL_IMPORT db::logging::Category* DB_MAIL_CAT;
DLL_IMPORT db::logging::Category* DB_MODEST_CAT;
DLL_IMPORT db::logging::Category* DB_NET_CAT;
DLL_IMPORT db::logging::Category* DB_RT_CAT;
DLL_IMPORT db::logging::Category* DB_SQL_CAT;
DLL_IMPORT db::logging::Category* DB_UTIL_CAT;

#undef DLL_IMPORT
#undef __DLL_IMPORT

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
