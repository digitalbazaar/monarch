/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/mysql/MySqlConnectionPool.h"

using namespace db::sql;
using namespace db::sql::mysql;

MySqlConnectionPool::MySqlConnectionPool(unsigned int poolSize) :
   AbstractConnectionPool(poolSize)
{
}

MySqlConnectionPool::~MySqlConnectionPool()
{
}

PooledConnection* MySqlConnectionPool::createConnection()
{
   return new PooledConnection(new MySqlConnection());
}
