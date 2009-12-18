/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/logging/LoggingCategories.h"

using namespace monarch::logging;

// DO NOT INITIALIZE THESE VARIABLES!
// These are not initialized on purpose due to initialization code issues.
Category* MO_APP_CAT;
Category* MO_CONFIG_CAT;
Category* MO_CRYPTO_CAT;
Category* MO_DATA_CAT;
Category* MO_EVENT_CAT;
Category* MO_GUI_CAT;
Category* MO_HTTP_CAT;
Category* MO_IO_CAT;
Category* MO_KERNEL_CAT;
Category* MO_LOGGING_CAT;
Category* MO_MAIL_CAT;
Category* MO_MODEST_CAT;
Category* MO_NET_CAT;
Category* MO_RT_CAT;
Category* MO_SPHINX_CAT;
Category* MO_SQL_CAT;
Category* MO_UPNP_CAT;
Category* MO_UTIL_CAT;

void LoggingCategories::initialize()
{
   MO_APP_CAT = new Category(
      "MO_APP",
      "MO Application",
      NULL);
   MO_CONFIG_CAT = new Category(
      "MO_CONFIG",
      "MO Config",
      NULL);
   MO_CRYPTO_CAT = new Category(
      "MO_CRYPTO",
      "MO Cryptography",
      NULL);
   MO_DATA_CAT = new Category(
      "MO_DATA",
      "MO Data",
      NULL);
   MO_EVENT_CAT = new Category(
      "MO_EVENT",
      "MO Event",
      NULL);
   MO_GUI_CAT = new Category(
      "MO_GUI",
      "MO Graphical User Interface",
      NULL);
   MO_HTTP_CAT = new Category(
      "MO_HTTP",
      "MO Http",
      NULL);
   MO_IO_CAT = new Category(
      "MO_IO",
      "MO Input/Output",
      NULL);
   MO_LOGGING_CAT = new Category(
      "MO_KERNEL",
      "MO Kernel",
      NULL);
   MO_LOGGING_CAT = new Category(
      "MO_LOGGING",
      "MO Logging",
      NULL);
   MO_MAIL_CAT = new Category(
      "MO_MAIL",
      "MO Mail",
      NULL);
   MO_MODEST_CAT = new Category(
      "MO_MODEST",
      "MO Modest Engine",
      NULL);
   MO_NET_CAT = new Category(
      "MO_NET",
      "MO Networking",
      NULL);
   MO_RT_CAT = new Category(
      "MO_RT",
      "MO Runtime",
      NULL);
   MO_SPHINX_CAT = new Category(
      "MO_SPHINX",
      "MO Sphinx",
      NULL);
   MO_SQL_CAT = new Category(
      "MO_SQL",
      "MO SQL",
      NULL);
   MO_UPNP_CAT = new Category(
      "MO_UPNP",
      "MO UPnP",
      NULL);
   MO_UTIL_CAT = new Category(
      "MO_UTIL",
      "MO Utilities",
      NULL);
}

void LoggingCategories::cleanup()
{
   delete MO_APP_CAT;
   MO_APP_CAT = NULL;

   delete MO_CONFIG_CAT;
   MO_CONFIG_CAT = NULL;

   delete MO_CRYPTO_CAT;
   MO_CRYPTO_CAT = NULL;

   delete MO_DATA_CAT;
   MO_DATA_CAT = NULL;

   delete MO_EVENT_CAT;
   MO_EVENT_CAT = NULL;

   delete MO_GUI_CAT;
   MO_GUI_CAT = NULL;

   delete MO_HTTP_CAT;
   MO_HTTP_CAT = NULL;

   delete MO_IO_CAT;
   MO_IO_CAT = NULL;

   delete MO_KERNEL_CAT;
   MO_KERNEL_CAT = NULL;

   delete MO_LOGGING_CAT;
   MO_LOGGING_CAT = NULL;

   delete MO_MAIL_CAT;
   MO_MAIL_CAT = NULL;

   delete MO_MODEST_CAT;
   MO_MODEST_CAT = NULL;

   delete MO_NET_CAT;
   MO_NET_CAT = NULL;

   delete MO_RT_CAT;
   MO_RT_CAT = NULL;

   delete MO_SPHINX_CAT;
   MO_SPHINX_CAT = NULL;

   delete MO_SQL_CAT;
   MO_SQL_CAT = NULL;

   delete MO_UPNP_CAT;
   MO_UPNP_CAT = NULL;

   delete MO_UTIL_CAT;
   MO_UTIL_CAT = NULL;
}
