/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/util/DatabaseManager.h"
#include "db/sql/SqlException.h"
#include "db/sql/mysql/MySqlConnectionPool.h"
#include "db/sql/sqlite3/Sqlite3ConnectionPool.h"
#include "db/net/Url.h"

using namespace std;
using namespace db::net;
using namespace db::sql;
using namespace db::sql::sqlite3;
using namespace db::sql::mysql;
using namespace db::sql::util;
using namespace db::rt;

DatabaseManager::DatabaseManager()
{
}

DatabaseManager::~DatabaseManager()
{
}

DatabaseClient* DatabaseManager::createClient(const char* url)
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
         ConnectionPool* cp = new MySqlConnectionPool(url);
         rval = new DatabaseClient(cp, true);
      }
      else if(strncmp(dbUrl.getScheme().c_str(), "sqlite3", 7) == 0)
      {
         // create sqlite3 connection pool for database client
         ConnectionPool* cp = new Sqlite3ConnectionPool(url);
         rval = new DatabaseClient(cp, true);
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
