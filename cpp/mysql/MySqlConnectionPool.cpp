/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/mysql/MySqlConnectionPool.h"

using namespace monarch::sql;
using namespace monarch::sql::mysql;

MySqlConnectionPool::MySqlConnectionPool(
   const char* url, unsigned int poolSize) :
   AbstractConnectionPool(url, poolSize),
   mNoEngineSubstitution(true)
{
}

MySqlConnectionPool::~MySqlConnectionPool()
{
}

PooledConnection* MySqlConnectionPool::createConnection()
{
   PooledConnection* rval = NULL;

   // create and connect connection
   MySqlConnection* c = new MySqlConnection();
   if(c->connect(&mUrl))
   {
      // handle engine substitution flag
      bool pass = true;
      if(mNoEngineSubstitution)
      {
         pass = c->setSqlMode("NO_ENGINE_SUBSTITUTION");
      }

      if(pass)
      {
         // wrap in a pooled connection
         rval = new PooledConnection(this, c);
      }
   }

   if(rval == NULL)
   {
      delete c;
   }

   return rval;
}

void MySqlConnectionPool::setNoEngineSubstitution(bool on)
{
   mNoEngineSubstitution = on;
}
