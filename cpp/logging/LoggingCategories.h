/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_logging_LoggingCategories_H
#define monarch_logging_LoggingCategories_H

#include "monarch/logging/Category.h"

// This is a list of pre-defined logging categories that can be used by any
// module that would like to log messages to a specific category.

extern monarch::logging::Category* DB_APP_CAT;
extern monarch::logging::Category* DB_CONFIG_CAT;
extern monarch::logging::Category* DB_CRYPTO_CAT;
extern monarch::logging::Category* DB_DATA_CAT;
extern monarch::logging::Category* DB_EVENT_CAT;
extern monarch::logging::Category* DB_GUI_CAT;
extern monarch::logging::Category* DB_HTTP_CAT;
extern monarch::logging::Category* DB_IO_CAT;
extern monarch::logging::Category* DB_LOGGING_CAT;
extern monarch::logging::Category* DB_MAIL_CAT;
extern monarch::logging::Category* DB_MODEST_CAT;
extern monarch::logging::Category* DB_NET_CAT;
extern monarch::logging::Category* DB_RT_CAT;
extern monarch::logging::Category* DB_SPHINX_CAT;
extern monarch::logging::Category* DB_SQL_CAT;
extern monarch::logging::Category* DB_UPNP_CAT;
extern monarch::logging::Category* DB_UTIL_CAT;

namespace monarch
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
} // end namespace monarch

#endif
