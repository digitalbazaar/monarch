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
   PooledConnection* rval = NULL;
   
   // create and connect connection
   Connection* c = new MySqlConnection();
   if(c->connect(&mUrl) == NULL)
   {
      // wrap in a pooled connection
      rval = new PooledConnection(c);
   }
   
   return rval;
}
