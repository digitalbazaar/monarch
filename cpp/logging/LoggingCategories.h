/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_logging_LoggingCategories_H
#define monarch_logging_LoggingCategories_H

#include "monarch/logging/Category.h"

// This is a list of pre-defined logging categories that can be used by any
// module that would like to log messages to a specific category.

extern monarch::logging::Category* MO_APP_CAT;
extern monarch::logging::Category* MO_CONFIG_CAT;
extern monarch::logging::Category* MO_CRYPTO_CAT;
extern monarch::logging::Category* MO_DATA_CAT;
extern monarch::logging::Category* MO_EVENT_CAT;
extern monarch::logging::Category* MO_GUI_CAT;
extern monarch::logging::Category* MO_HTTP_CAT;
extern monarch::logging::Category* MO_IO_CAT;
extern monarch::logging::Category* MO_KERNEL_CAT;
extern monarch::logging::Category* MO_LOGGING_CAT;
extern monarch::logging::Category* MO_MAIL_CAT;
extern monarch::logging::Category* MO_MODEST_CAT;
extern monarch::logging::Category* MO_NET_CAT;
extern monarch::logging::Category* MO_RT_CAT;
extern monarch::logging::Category* MO_SPHINX_CAT;
extern monarch::logging::Category* MO_SQL_CAT;
extern monarch::logging::Category* MO_UPNP_CAT;
extern monarch::logging::Category* MO_UTIL_CAT;

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
