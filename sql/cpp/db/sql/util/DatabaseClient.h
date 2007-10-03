/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_util_DatabaseClient_H
#define db_sql_util_DatabaseClient_H

#include "db/sql/Connection.h"
#include "db/sql/ConnectionPool.h"

namespace db
{
namespace sql
{
namespace util
{

/**
 * A DatabaseClient maintains a ConnectionPool of connections to a particular
 * database. Upon request it can provide a connection for executing SQL
 * statements.
 * 
 * @author Dave Longley
 */
class DatabaseClient
{
protected:
   /**
    * The ConnectionPool used by this DatabaseClient.
    */
   db::sql::ConnectionPool* mConnectionPool;
   
   /**
    * True if this client should clean up its connection pool, false if not.
    */
   bool mCleanupPool;
   
public:
   /**
    * Creates a new DatabaseClient with the specified ConnectionPool.
    * 
    * @param cp the ConnectionPool to use.
    * @param cleanup true if the ConnectionPool should be cleaned up upon
    *                destruction of this DatabastClient, false if not.
    */
   DatabaseClient(db::sql::ConnectionPool* cp, bool cleanup);
   
   /**
    * Destructs this DatabaseClient.
    */
   virtual ~DatabaseClient();
   
   /**
    * Gets a Connection from the ConnectionPool.
    * 
    * @return a Connection from the ConnectionPool.
    */
   virtual db::sql::Connection* getConnection();
   
   /**
    * Provides access to this DatabastClient's ConnectionPool if changes to
    * it are desired.
    */
   virtual ConnectionPool* getConnectionPool();
};

} // end namespace util
} // end namespace sql
} // end namespace db
#endif
