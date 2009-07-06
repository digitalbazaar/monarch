/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_PooledConnection_H
#define db_database_PooledConnection_H

#include "db/sql/Connection.h"
#include "db/sql/SqlException.h"

namespace db
{
namespace sql
{

// forward declare abstract connection pool
class AbstractConnectionPool;

/**
 * A PooledConnection wraps an existing Connection and adds an idle
 * timestamp and lazy expiration functionality.
 * 
 * @author Mike Johnson
 */
class PooledConnection : public Connection
{
protected:
   /**
    * The abstract connection pool that owns this pooled connection.
    */
   AbstractConnectionPool* mPool;
   
   /**
    * The wrapped connection.
    */
   Connection* mConnection;
   
   /**
    * Stores the last time in milliseconds that the connection went idle.
    */
   uint64_t mIdleTime;
   
public:
   /**
    * Creates a new PooledConnection around the passed Connection.
    * 
    * @param pool the connection pool this connection belongs to.
    * @param connection the wrapped connection.
    */
   PooledConnection(AbstractConnectionPool* pool, Connection* connection);
   
   /**
    * Destructs this PooledConnection.
    */
   virtual ~PooledConnection();
   
   /**
    * Gets the wrapped connection.
    * 
    * @return a pointer to the wrapped connection.
    */
   virtual Connection* getConnection();
   
   /**
    * Sets the last time the connection went idle. Setting to 0 indicates
    * connection is active.
    * 
    * @param idleTime the time in milliseconds.
    */
   virtual void setIdleTime(uint64_t idleTime);
   
   /**
    * Gets the last time the connection went idle. Idle time of 0 indicates
    * connection is active.
    * 
    * @return the time in milliseconds that connection went idle,
    *         0 if connection is active.
    */
   virtual uint64_t getIdleTime();
   
   /**
    * Connects to the database specified by the given url.
    * 
    * @param url the url for the database to connect to, including driver
    *            specific parameters.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool connect(const char* url);
   
   /**
    * Connects to the database specified by the given url.
    * 
    * @param url the url for the database to connect to, including driver
    *            specific parameters.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool connect(db::net::Url* url);
   
   /**
    * Prepares a Statement for execution. The Statement, if valid, is stored
    * along with the Connection according to its sql. It's memory is handled
    * internally.
    * 
    * @param sql the standard query language text of the Statement.
    * 
    * @return the new stored Statement, NULL if an exception occurred.
    */
   virtual Statement* prepare(const char* sql);
   
   /**
    * Faux closes this database connection. Sets the idle time for this
    * connection so that the connection may be reused or shut down.
    */
   virtual void close();
   
   /**
    * Closes this pooled connection. This shuts down the underlying connection
    * and *must* only be called by a parent ConnectionPool.
    */
   virtual void closeConnection();
   
   /**
    * Begins a new transaction.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool begin();
   
   /**
    * Commits the current transaction.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool commit();
   
   /**
    * Rolls back the current transaction.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool rollback();
   
   /**
    * Returns true if this connection is connected, false if not.
    * 
    * @return true if this connection is connected, false if not.
    */
   virtual bool isConnected();
   
   /**
    * Cleans up this connection's prepared statements.
    */
   virtual void cleanupPreparedStatements();
};

} // end namespace sql
} // end namespace db

#endif
