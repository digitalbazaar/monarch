/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/PooledConnection.h"
#include "db/sql/AbstractConnectionPool.h"

using namespace db::net;
using namespace db::rt;
using namespace db::sql;

PooledConnection::PooledConnection(
   AbstractConnectionPool* pool, Connection* connection)
{
   mPool = pool;
   mConnection = connection;
}

PooledConnection::~PooledConnection()
{
   // clean up wrapped connection
   if(mConnection != NULL)
   {
      delete mConnection;
   }
}

inline Connection* PooledConnection::getConnection()
{
   return mConnection;
}

inline void PooledConnection::closeConnection()
{
   // call real connection close
   mConnection->close();
}

inline Statement* PooledConnection::createStatement(const char* sql)
{
   // pooled connections don't create their own statements
   return NULL;
}

inline void PooledConnection::setIdleTime(uint64_t idleTime)
{
   mIdleTime = idleTime;
}

inline uint64_t PooledConnection::getIdleTime()
{
   return mIdleTime;
}

inline bool PooledConnection::connect(Url* url)
{
   return mConnection->connect(url);
}

inline Statement* PooledConnection::prepare(const char* sql)
{
   return mConnection->prepare(sql);
}

inline void PooledConnection::close()
{
   // don't close the connection, instead notify the pool that this
   // connection is now idle
   mPool->connectionClosed(this);
}

inline bool PooledConnection::begin()
{
   return mConnection->begin();
}

inline bool PooledConnection::commit()
{
   return mConnection->commit();
}

inline bool PooledConnection::rollback()
{
   return mConnection->rollback();
}

inline bool PooledConnection::isConnected()
{
   return mConnection->isConnected();
}
