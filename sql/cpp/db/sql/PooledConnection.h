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
    * Friend of AbstractConnectionPool to allow access to protected close
    * connection.
    */
   friend class AbstractConnectionPool;
   
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
   unsigned long long mIdleTime;
   
   /**
    * Closes this pooled connection.
    */
   virtual void closeConnection();
   
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
   virtual void setIdleTime(unsigned long long idleTime);
   
   /**
    * Gets the last time the connection went idle. Idle time of 0 indicates
    * connection is active.
    * 
    * @return the time in milliseconds that connection went idle,
    *         0 if connection is active.
    */
   virtual unsigned long long getIdleTime();
   
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
    * Prepares a Statement for execution. The Statement is heap-allocated and
    * must be freed by the caller of this method.
    * 
    * @param sql the standard query language text of the Statement.
    * 
    * @return the new Statement to be freed by caller, NULL if an
    *         exception occurred.
    */
   virtual Statement* prepare(const char* sql);
   
   /**
    * Faux closes this database connection. Sets the idle time for this
    * connection so that the connection may be reused or shut down.
    */
   virtual void close();
   
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
};

} // end namespace sql
} // end namespace db

#endif
