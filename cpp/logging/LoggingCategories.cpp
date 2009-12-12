/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/logging/LoggingCategories.h"

using namespace monarch::logging;

// DO NOT INITIALIZE THESE VARIABLES!
// These are not initialized on purpose due to initialization code issues.
Category* DB_APP_CAT;
Category* DB_CONFIG_CAT;
Category* DB_CRYPTO_CAT;
Category* DB_DATA_CAT;
Category* DB_EVENT_CAT;
Category* DB_GUI_CAT;
Category* DB_HTTP_CAT;
Category* DB_IO_CAT;
Category* DB_LOGGING_CAT;
Category* DB_MAIL_CAT;
Category* DB_MODEST_CAT;
Category* DB_NET_CAT;
Category* DB_RT_CAT;
Category* DB_SPHINX_CAT;
Category* DB_SQL_CAT;
Category* DB_UPNP_CAT;
Category* DB_UTIL_CAT;

void LoggingCategories::initialize()
{
   DB_APP_CAT = new Category(
      "DB_APP",
      "DB Application",
      NULL);
   DB_CONFIG_CAT = new Category(
      "DB_CONFIG",
      "DB Config",
      NULL);
   DB_CRYPTO_CAT = new Category(
      "DB_CRYPTO",
      "DB Cryptography",
      NULL);
   DB_DATA_CAT = new Category(
      "DB_DATA",
      "DB Data",
      NULL);
   DB_EVENT_CAT = new Category(
      "DB_EVENT",
      "DB Event",
      NULL);
   DB_GUI_CAT = new Category(
      "DB_GUI",
      "DB Graphical User Interface",
      NULL);
   DB_HTTP_CAT = new Category(
      "DB_HTTP",
      "DB Http",
      NULL);
   DB_IO_CAT = new Category(
      "DB_IO",
      "DB Input/Output",
      NULL);
   DB_LOGGING_CAT = new Category(
      "DB_LOGGING",
      "DB Logging",
      NULL);
   DB_MAIL_CAT = new Category(
      "DB_MAIL",
      "DB Mail",
      NULL);
   DB_MODEST_CAT = new Category(
      "DB_MODEST",
      "DB Modest Engine",
      NULL);
   DB_NET_CAT = new Category(
      "DB_NET",
      "DB Networking",
      NULL);
   DB_RT_CAT = new Category(
      "DB_RT",
      "DB Runtime",
      NULL);
   DB_SPHINX_CAT = new Category(
      "DB_SPHINX",
      "DB Sphinx",
      NULL);
   DB_SQL_CAT = new Category(
      "DB_SQL",
      "DB SQL",
      NULL);
   DB_UPNP_CAT = new Category(
      "DB_UPNP",
      "DB UPnP",
      NULL);
   DB_UTIL_CAT = new Category(
      "DB_UTIL",
      "DB Utilities",
      NULL);
}

void LoggingCategories::cleanup()
{
   delete DB_APP_CAT;
   DB_APP_CAT = NULL;

   delete DB_CONFIG_CAT;
   DB_CONFIG_CAT = NULL;

   delete DB_CRYPTO_CAT;
   DB_CRYPTO_CAT = NULL;

   delete DB_DATA_CAT;
   DB_DATA_CAT = NULL;

   delete DB_EVENT_CAT;
   DB_EVENT_CAT = NULL;

   delete DB_GUI_CAT;
   DB_GUI_CAT = NULL;

   delete DB_HTTP_CAT;
   DB_HTTP_CAT = NULL;

   delete DB_IO_CAT;
   DB_IO_CAT = NULL;

   delete DB_LOGGING_CAT;
   DB_LOGGING_CAT = NULL;

   delete DB_MAIL_CAT;
   DB_MAIL_CAT = NULL;

   delete DB_MODEST_CAT;
   DB_MODEST_CAT = NULL;

   delete DB_NET_CAT;
   DB_NET_CAT = NULL;

   delete DB_RT_CAT;
   DB_RT_CAT = NULL;

   delete DB_SPHINX_CAT;
   DB_SPHINX_CAT = NULL;

   delete DB_SQL_CAT;
   DB_SQL_CAT = NULL;

   delete DB_UPNP_CAT;
   DB_UPNP_CAT = NULL;

   delete DB_UTIL_CAT;
   DB_UTIL_CAT = NULL;
}
