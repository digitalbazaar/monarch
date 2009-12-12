/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_ConnectionPool_H
#define monarch_sql_ConnectionPool_H

#include "monarch/sql/PooledConnection.h"
#include "monarch/rt/Collectable.h"

namespace monarch
{
namespace sql
{

/**
 * A ConnectionPool provides an interface for a pool of connections to a
 * particular database. Pooling connections allows for a quicker and more
 * efficient use of resources.
 *
 * @author Dave Longley
 */
class ConnectionPool
{
public:
   /**
    * Creates a new ConnectionPool.
    */
   ConnectionPool() {};

   /**
    * Destructs this ConnectionPool.
    */
   virtual ~ConnectionPool() {};

   /**
    * Gets a Connection from this connection pool to use to execute
    * statements. The Connection should be closed when it is no longer
    * needed. Closing the Connection will return control over it back to
    * this connection pool.
    *
    * @return a Connection from this connection pool, or NULL if an exception
    *         occurred.
    */
   virtual Connection* getConnection() = 0;

   /**
    * Closes all connections.
    */
   virtual void closeAllConnections() = 0;

   /**
    * Sets the number of connections in this connection pool. If a size of
    * 0 is specified, than there will be no limit to the number of
    * connections in this pool.
    *
    * @param size the number of connections in this connection pool. A size
    *             of 0 specifies an unlimited number of connections.
    */
   virtual void setPoolSize(unsigned int size) = 0;

   /**
    * Gets the number of connections in this connection pool. If a size of
    * 0 is returned, than there is no limit to the number of connections
    * in this pool.
    *
    * @return the number of connections in this connection pool. A size
    *         of 0 specifies an unlimited number of connections.
    */
   virtual unsigned int getPoolSize() = 0;

   /**
    * Sets the expire time for all connections.
    *
    * @param expireTime the amount of time that must pass while connections
    *                   are idle in order for them to expire -- if 0 is passed
    *                   then connections will never expire.
    */
   virtual void setConnectionExpireTime(unsigned long long expireTime) = 0;

   /**
    * Gets the expire time for all connections.
    *
    * @return the expire time for all connections.
    */
   virtual unsigned long long getConnectionExpireTime() = 0;

   /**
    * Gets the current number of connections in the pool.
    *
    * @return the current number of connections in the pool.
    */
   virtual unsigned int getConnectionCount() = 0;

   /**
    * Gets the current number of active connections.
    *
    * @return the current number of active connections.
    */
   virtual unsigned int getActiveConnectionCount() = 0;

   /**
    * Gets the current number of idle connections.
    *
    * @return the current number of idle connections.
    */
   virtual unsigned int getIdleConnectionCount() = 0;

   /**
    * Gets the current number of expired connections.
    *
    * @return the current number of expired connections.
    */
   virtual unsigned int getExpiredConnectionCount() = 0;
};

// type definition for a reference counted ConnectionPool
typedef monarch::rt::Collectable<ConnectionPool> ConnectionPoolRef;

} // end namespace sql
} // end namespace monarch
#endif
