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
   // create and connect connection
   Connection* c = new Sqlite3Connection();
   c->connect(&mUrl);
   
   // wrap in a pooled connection
   return new PooledConnection(c);
}
