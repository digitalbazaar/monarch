/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/PooledConnection.h"

#include "monarch/sql/AbstractConnectionPool.h"

using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::sql;

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

inline void PooledConnection::setIdleTime(uint64_t idleTime)
{
   mIdleTime = idleTime;
}

inline uint64_t PooledConnection::getIdleTime()
{
   return mIdleTime;
}

inline bool PooledConnection::connect(const char* url)
{
   return mConnection->connect(url);
}

inline bool PooledConnection::connect(Url* url)
{
   return mConnection->connect(url);
}

inline Statement* PooledConnection::prepare(const char* sql)
{
   return mConnection->prepare(sql);
}

Statement* PooledConnection::preparef(const char* format, ...)
{
   Statement* rval = NULL;

   va_list ap;
   va_start(ap, format);
   rval = vpreparef(format, ap);
   va_end(ap);

   return rval;
}

Statement* PooledConnection::vpreparef(const char* format, va_list varargs)
{
   return mConnection->vpreparef(format, varargs);
}

inline void PooledConnection::close()
{
   // don't close the connection, instead notify the pool that this
   // connection is now idle
   mPool->connectionClosed(this);
}

inline void PooledConnection::closeConnection()
{
   // call real connection close
   mConnection->close();
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

inline void PooledConnection::cleanupPreparedStatements()
{
   mConnection->cleanupPreparedStatements();
}
