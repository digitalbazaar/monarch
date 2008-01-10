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
      "db:config",
      "DB configuration module");
   DB_CRYPTO_CAT = new Category(
      "DB Cryptography",
      "db:crypto",
      "DB cryptography module");
   DB_DATA_CAT = new Category(
      "DB Data",
      "db:data",
      NULL);
   DB_EVENT_CAT = new Category(
      "DB Event",
      "db:event",
      NULL);
   DB_GUI_CAT = new Category(
      "DB Graphical User Interface",
      "db:gui",
      NULL);
   DB_IO_CAT = new Category(
      "DB Input/Output",
      "db:io",
      NULL);
   DB_LOGGING_CAT = new Category(
      "DB Logging",
      "db:logging",
      NULL);
   DB_MAIL_CAT = new Category(
      "DB Mail",
      "db:mail",
      NULL);
   DB_MODEST_CAT = new Category(
      "DB Modest Engine",
      "db:modest",
      NULL);
   DB_NET_CAT = new Category(
      "DB Networking",
      "db:net",
      NULL);
   DB_RT_CAT = new Category(
      "DB Runtime",
      "db:rt",
      NULL);
   DB_SQL_CAT = new Category(
      "DB SQL",
      "db:sql",
      NULL);
   DB_UTIL_CAT = new Category(
      "DB Utilities",
      "db:util",
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
