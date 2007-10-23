/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/util/DatabaseClient.h"
#include "db/net/Url.h"
#include "db/sql/mysql/MySqlConnectionPool.h"
#include "db/sql/sqlite3/Sqlite3ConnectionPool.h"

using namespace db::net;
using namespace db::rt;
using namespace db::sql;
using namespace db::sql::mysql;
using namespace db::sql::sqlite3;
using namespace db::sql::util;

DatabaseClient::DatabaseClient(ConnectionPool* cp, bool cleanup)
{
   mConnectionPool = cp;
   mCleanupPool = cleanup;
}

DatabaseClient::~DatabaseClient()
{
   if(mCleanupPool)
   {
      delete mConnectionPool;
   }
}

Connection* DatabaseClient::getConnection()
{
   return mConnectionPool->getConnection();
}

ConnectionPool* DatabaseClient::getConnectionPool()
{
   return mConnectionPool;
}

DatabaseClient* DatabaseClient::create(const char* url)
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
         char temp[dbUrl.getScheme().length() + 100];
         sprintf(
            temp, "Cannot create DatabaseClient, url scheme is "
            "not recognized!,scheme='%s'", dbUrl.getScheme().c_str());
         Exception::setLast(new SqlException(temp));
      }
   }
   
   return rval;
}
