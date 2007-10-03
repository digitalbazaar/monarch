/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/PooledConnection.h"

using namespace db::rt;
using namespace db::sql;

PooledConnection::PooledConnection(Connection* connection)
{
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

SqlException* PooledConnection::connect(const char* url)
{
   return mConnection->connect(url);
}

Statement* PooledConnection::prepare(const char* sql)
{
   return mConnection->prepare(sql);
}

void PooledConnection::close()
{
   // set the idle time instead of closing connection
   setIdleTime(System::getCurrentMilliseconds());
}

SqlException* PooledConnection::commit()
{
   return mConnection->commit();
}

SqlException* PooledConnection::rollback()
{
   return mConnection->rollback();
}
