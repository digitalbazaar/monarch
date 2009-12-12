/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_sqlite3_Sqlite3ConnectionPool_H
#define monarch_sql_sqlite3_Sqlite3ConnectionPool_H

#include "monarch/sql/AbstractConnectionPool.h"
#include "monarch/sql/sqlite3/Sqlite3Connection.h"

namespace monarch
{
namespace sql
{
namespace sqlite3
{

/**
 * The Sqlite3 connection pool manages active, idle and expired Sqlite3
 * database connections.
 *
 * @author Mike Johnson
 */
class Sqlite3ConnectionPool : public AbstractConnectionPool
{
protected:
   /**
    * Creates a new sqlite3 database connection, connects it, and wraps it
    * with a PooledConnection.
    *
    * @return the PooledConnection or NULL if an exception occurred.
    */
   virtual PooledConnection* createConnection();

public:
   /**
    * Creates a new Sqlite3ConnectionPool with the specified number of
    * database connections available.
    *
    * @param url the url for the database connections, including driver
    *            specific parameters.
    * @param poolSize the size of the pool (number of database connections),
    *                 0 specifies an unlimited number of threads.
    */
   Sqlite3ConnectionPool(const char* url, unsigned int poolSize = 10);

   /**
    * Destructs this Sqlite3ConnectionPool.
    */
   virtual ~Sqlite3ConnectionPool();
};

} // end namespace sqlite3
} // end namespace sql
} // end namespace monarch

#endif
