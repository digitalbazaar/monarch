/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/util/DatabaseManager.h"
#include "db/sql/SqlException.h"
#include "db/net/Url.h"

using namespace std;
using namespace db::net;
using namespace db::sql;
using namespace db::sql::util;
using namespace db::rt;

DatabaseManager::DatabaseManager()
{
}

DatabaseManager::~DatabaseManager()
{
}

DatabaseClient* DatabaseManager::createDatabaseClient(const char* url)
{
   DatabaseClient* rval = NULL;
   
   // parse url, ensure it isn't malformed
   Exception::clearLast();
   Url dbUrl(url);
   if(!Exception::hasLast())
   {
      if(strncmp(dbUrl.getScheme().c_str(), "mysql", 5) == 0)
      {
         // create mysql connection pool for database client
         // FIXME:
      }
      else if(strncmp(dbUrl.getScheme().c_str(), "sqlite3", 7) == 0)
      {
         // create sqlite3 connection pool for database client
         // FIXME:
      }
      else
      {
         string msg;
         msg.append("Cannot create DatabaseClient, ");
         msg.append("url scheme is not recognized!,scheme='");
         msg.append(dbUrl.getScheme());
         msg.append(1, '\'');
         Exception::setLast(new SqlException(msg.c_str()));
      }
   }
   
   return rval;
}
