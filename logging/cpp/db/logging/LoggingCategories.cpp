/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/logging/LoggingCategories.h"

using namespace db::logging;
 
// DO NOT INITIALIZE THESE VARIABLES!
// These are not initialized on purpose due to initialization code issues.
Category* DB_CONFIG_CAT;
Category* DB_CRYPTO_CAT; 
Category* DB_DATA_CAT;
Category* DB_EVENT_CAT;
Category* DB_GUI_CAT;
Category* DB_IO_CAT;
Category* DB_LOGGING_CAT;
Category* DB_MAIL_CAT;
Category* DB_MODEST_CAT;
Category* DB_NET_CAT;
Category* DB_RT_CAT;
Category* DB_SQL_CAT;
Category* DB_UTIL_CAT;

void LoggingCategories::initialize()
{
   DB_CONFIG_CAT = new Category(
      "DB Config",
      "DB_CONFIG",
      NULL);
   DB_CRYPTO_CAT = new Category(
      "DB Cryptography",
      "DB_CRYPTO",
      NULL);
   DB_DATA_CAT = new Category(
      "DB Data",
      "DB_DATA",
      NULL);
   DB_EVENT_CAT = new Category(
      "DB Event",
      "DB_EVENT",
      NULL);
   DB_GUI_CAT = new Category(
      "DB Graphical User Interface",
      "DB_GUI",
      NULL);
   DB_IO_CAT = new Category(
      "DB Input/Output",
      "DB_IO",
      NULL);
   DB_LOGGING_CAT = new Category(
      "DB Logging",
      "DB_LOGGING",
      NULL);
   DB_MAIL_CAT = new Category(
      "DB Mail",
      "DB_MAIL",
      NULL);
   DB_MODEST_CAT = new Category(
      "DB Modest Engine",
      "DB_MODEST",
      NULL);
   DB_NET_CAT = new Category(
      "DB Networking",
      "DB_NET",
      NULL);
   DB_RT_CAT = new Category(
      "DB Runtime",
      "DB_RT",
      NULL);
   DB_SQL_CAT = new Category(
      "DB SQL",
      "DB_SQL",
      NULL);
   DB_UTIL_CAT = new Category(
      "DB Utilities",
      "DB_UTIL",
      NULL);
}

void LoggingCategories::cleanup()
{
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
   
   delete DB_SQL_CAT;
   DB_SQL_CAT = NULL;
   
   delete DB_UTIL_CAT;
   DB_UTIL_CAT = NULL;
}
