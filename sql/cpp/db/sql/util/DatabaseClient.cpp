/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/util/DatabaseClient.h"

using namespace db::sql;
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
