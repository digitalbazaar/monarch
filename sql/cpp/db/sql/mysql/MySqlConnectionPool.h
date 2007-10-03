/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_mysql_MySqlConnectionPool_H
#define db_sql_mysql_MySqlConnectionPool_H

#include "db/sql/AbstractConnectionPool.h"
#include "db/sql/mysql/MySqlConnection.h"

namespace db
{
namespace sql
{
namespace mysql
{

/**
 * The MySql connection pool manages active, idle and expired MySql database
 * connections.
 * 
 * @author Mike Johnson
 */
class MySqlConnectionPool : public AbstractConnectionPool
{
protected:
   /**
    * Creates a new MySql database connection in a PooledConnection.
    */
   virtual PooledConnection* createConnection();

public:
   /**
    * Creates a new MySqlConnectionPool with the specified number of
    * database connections available.
    * 
    * @param url the url for the database connections, including driver
    *            specific parameters.
    * @param poolSize the size of the pool (number of database connections),
    *                 0 specifies an unlimited number of threads.
    */
   MySqlConnectionPool(const char* url, unsigned int poolSize = 10);
   
   /**
    * Destructs this MySqlConnectionPool.
    */
   virtual ~MySqlConnectionPool();
};

} // end namespace mysql
} // end namespace sql
} // end namespace db

#endif
