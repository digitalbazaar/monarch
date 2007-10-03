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
   
public:
   /**
    * Creates a new DatabaseClient with the specified ConnectionPool.
    * 
    * @param cp the ConnectionPool to use.
    */
   DatabaseClient(db::sql::ConnectionPool* cp);
   
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
};

} // end namespace util
} // end namespace sql
} // end namespace db
#endif
