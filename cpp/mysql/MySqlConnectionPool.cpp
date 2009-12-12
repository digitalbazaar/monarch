/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/mysql/MySqlConnectionPool.h"

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
   if(c->connect(&mUrl))
   {
      // wrap in a pooled connection
      rval = new PooledConnection(this, c);
   }

   return rval;
}
