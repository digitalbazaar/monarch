/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/sqlite3/Sqlite3ConnectionPool.h"

using namespace db::sql;
using namespace db::sql::sqlite3;

Sqlite3ConnectionPool::Sqlite3ConnectionPool(
   const char* url, unsigned int poolSize) :
   AbstractConnectionPool(url, poolSize)
{
}

Sqlite3ConnectionPool::~Sqlite3ConnectionPool()
{
}

PooledConnection* Sqlite3ConnectionPool::createConnection()
{
   return new PooledConnection(new Sqlite3Connection());
}
