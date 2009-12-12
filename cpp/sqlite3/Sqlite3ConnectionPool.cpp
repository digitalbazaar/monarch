/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/sqlite3/Sqlite3ConnectionPool.h"

using namespace monarch::sql;
using namespace monarch::sql::sqlite3;

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
   PooledConnection* rval = NULL;

   // create and connect connection
   Connection* c = new Sqlite3Connection();
   if(c->connect(&mUrl))
   {
      // wrap in a pooled connection
      rval = new PooledConnection(this, c);
   }

   return rval;
}
