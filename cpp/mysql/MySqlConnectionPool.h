/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_mysql_MySqlConnectionPool_H
#define monarch_sql_mysql_MySqlConnectionPool_H

#include "monarch/sql/AbstractConnectionPool.h"
#include "monarch/sql/mysql/MySqlConnection.h"

namespace monarch
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
    * MySql NO_ENGINE_SUBSTITUTION flag.
    */
   bool mNoEngineSubstitution;

   /**
    * MySql timezone setting.
    */
   char* mTimeZone;

   /**
    * Creates a new mysql database connection, connects it, and wraps it with a
    * PooledConnection.
    *
    * @return the PooledConnection or NULL if an exception occurred.
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

   /**
    * Sets whether or not MYSQL ENGINE substitution is disabled on the
    * connections created by this pool. It is disabled by default, which
    * means that if an ENGINE is specifed in a CREATE or ALTER TABLE statement,
    * and that ENGINE is not available, the statement will fail.
    *
    * @param on true to disallow ENGINE substitution when creating or altering
    *           tables if the chosen ENGINE fails (default), false to allow it.
    */
   virtual void setNoEngineSubstitution(bool on);

   /**
    * Sets the default timezone for connections.
    *
    * @param tz the default timezone for new connections.
    */
   virtual void setTimeZone(const char* tz);
};

} // end namespace mysql
} // end namespace sql
} // end namespace monarch

#endif
