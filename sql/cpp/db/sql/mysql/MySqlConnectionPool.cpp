/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/mysql/MySqlConnectionPool.h"

using namespace db::sql;
using namespace db::sql::mysql;

MySqlConnectionPool::MySqlConnectionPool(
   const char* url, unsigned int poolSize) :
   AbstractConnectionPool(url, poolSize)
{
}

MySqlConnectionPool::~MySqlConnectionPool()
{
}

PooledConnection* MySqlConnectionPool::createConnection()
{
   // create and connect connection
   Connection* c = new MySqlConnection();
   c->connect(&mUrl);
   
   // wrap in a pooled connection
   return new PooledConnection(c);
}
