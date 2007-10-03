/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_sqlite3_Sqlite3ConnectionPool_H
#define db_sql_sqlite3_Sqlite3ConnectionPool_H

#include "db/sql/AbstractConnectionPool.h"
#include "db/sql/sqlite3/Sqlite3Connection.h"

namespace db
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
    * Creates a new Sqlite3 database connection in a PooledConnection.
    */
   virtual PooledConnection* createConnection();
   
public:
   /**
    * Creates a new Sqlite3ConnectionPool with the specified number of
    * database connections available.
    * 
    * @param poolSize the size of the pool (number of database connections),
    *                 0 specifies an unlimited number of threads.
    */
   Sqlite3ConnectionPool(unsigned int poolSize = 10);
   
   /**
    * Destructs this Sqlite3ConnectionPool.
    */
   virtual ~Sqlite3ConnectionPool();
};

} // end namespace sqlite3
} // end namespace sql
} // end namespace db

#endif
