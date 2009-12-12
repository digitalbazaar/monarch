/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_sql_AbstractConnectionPool_H
#define db_sql_AbstractConnectionPool_H

#include "monarch/net/Url.h"
#include "monarch/rt/ExclusiveLock.h"
#include "monarch/rt/Semaphore.h"
#include "monarch/rt/System.h"
#include "monarch/sql/ConnectionPool.h"
#include "monarch/sql/PooledConnection.h"

#include <list>

namespace db
{
namespace sql
{

/**
 * The abstract connection pool provides the basic implementation for a
 * connection pool. This is the base class for connection pools that have
 * specific database connection types.
 *
 * This pool maintains a set of N connections to the database. Any new
 * connections are lazily created.
 *
 * @author Mike Johnson
 */
class AbstractConnectionPool :
public virtual db::rt::ExclusiveLock, public ConnectionPool
{
protected:
   /**
    * Friend of PooledConnection to allow access to protected close
    * connection.
    */
   friend class PooledConnection;

   /**
    * This semaphore is used to regulate the number of connections that
    * are available in this pool.
    */
   db::rt::Semaphore mConnectionSemaphore;

   /**
    * The list of active connections in this pool.
    */
   std::list<PooledConnection*> mActiveConnections;

   /**
    * The list of idle connections in this pool.
    */
   std::list<PooledConnection*> mIdleConnections;

   /**
    * A lock for modifying the connection lists.
    */
   db::rt::ExclusiveLock mListLock;

   /**
    * The database driver parameters in URL form for creating database
    * connections.
    */
   db::net::Url mUrl;

   /**
    * The expire time for Connections (in milliseconds).
    */
   uint64_t mConnectionExpireTime;

   /**
    * Creates a new database connection, connects it, and wraps it with a
    * PooledConnection.
    *
    * @return the PooledConnection or NULL if an exception occurred.
    */
   virtual PooledConnection* createConnection() = 0;

   /**
    * Notifies the pool that a specific connection has been closed and is
    * now idle.
    *
    * @param connection the PooledConnection that has been closed.
    */
   virtual void connectionClosed(PooledConnection* connection);

   /**
    * Gets an idle connection. If an idle connection is not found, it will be
    * created as long as the pool size allows it.
    *
    * In addition to removing expired connections, this method will also remove
    * any extra idle connections that should not exist due to a decrease in the
    * pool size.
    *
    * @return an idle database connection downcase to a base connection.
    */
   virtual Connection* getIdleConnection();

   /**
    * Closes all expired connections.
    */
   virtual void closeExpiredConnections();

public:
   /**
    * Creates a new AbstractConnectionPool with the specified number of
    * database connections available.
    *
    * @param url the url for the database connections, including driver
    *            specific parameters.
    * @param poolSize the size of the pool (number of database connections),
    *                 0 specifies an unlimited number of connections.
    */
   AbstractConnectionPool(const char* url, unsigned int poolSize = 10);

   /**
    * Destructs this AbstractConnectionPool.
    */
   virtual ~AbstractConnectionPool();

   /**
    * Gets a Connection from this connection pool to use to execute
    * statements. The Connection should be closed when it is no longer
    * needed. Closing the Connection will return control over it back to
    * this connection pool.
    *
    * @return a Connection from this connection pool, or NULL if an exception
    *         occurred.
    */
   virtual Connection* getConnection();

   /**
    * Closes all connections.
    */
   virtual void closeAllConnections();

   /**
    * Sets the number of connections in this connection pool. If a size of
    * 0 is specified, than there will be no limit to the number of
    * connections in this pool.
    *
    * @param size the number of connections in this connection pool. A size
    *             of 0 specifies an unlimited number of connections.
    */
   virtual void setPoolSize(unsigned int size);

   /**
    * Gets the number of connections in this connection pool. If a size of
    * 0 is returned, than there is no limit to the number of connections
    * in this pool.
    *
    * @return the number of connections in this connection pool. A size
    *         of 0 specifies an unlimited number of connections.
    */
   virtual unsigned int getPoolSize();

   /**
    * Sets the expire time for all connections.
    *
    * @param expireTime the amount of time that must pass while connections
    *                   are idle in order for them to expire -- if 0 is passed
    *                   then connections will never expire.
    */
   virtual void setConnectionExpireTime(uint64_t expireTime);

   /**
    * Gets the expire time for all connections.
    *
    * @return the expire time for all connections.
    */
   virtual uint64_t getConnectionExpireTime();

   /**
    * Gets the current number of connections in the pool.
    *
    * @return the current number of connections in the pool.
    */
   virtual unsigned int getConnectionCount();

   /**
    * Gets the current number of active connections.
    *
    * @return the current number of active connections.
    */
   virtual unsigned int getActiveConnectionCount();

   /**
    * Gets the current number of idle connections.
    *
    * @return the current number of idle connections.
    */
   virtual unsigned int getIdleConnectionCount();

   /**
    * Gets the current number of expired connections.
    *
    * @return the current number of expired connections.
    */
   virtual unsigned int getExpiredConnectionCount();
};

} // end namespace sql
} // end namespace db
#endif
