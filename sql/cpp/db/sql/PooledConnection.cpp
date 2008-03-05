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

Connection* PooledConnection::getConnection()
{
   return mConnection;
}

void PooledConnection::closeConnection()
{
   // call real connection close
   mConnection->close();
}

void PooledConnection::setIdleTime(unsigned long long idleTime)
{
   mIdleTime = idleTime;
}

unsigned long long PooledConnection::getIdleTime()
{
   return mIdleTime;
}

bool PooledConnection::connect(Url* url)
{
   return mConnection->connect(url);
}

Statement* PooledConnection::prepare(const char* sql)
{
   return mConnection->prepare(sql);
}

void PooledConnection::close()
{
   // don't close the connection, instead notify the pool that this
   // connection is now idle
   mPool->connectionClosed(this);
}

bool PooledConnection::begin()
{
   return mConnection->begin();
}

bool PooledConnection::commit()
{
   return mConnection->commit();
}

bool PooledConnection::rollback()
{
   return mConnection->rollback();
}

bool PooledConnection::isConnected()
{
   return mConnection->isConnected();
}
