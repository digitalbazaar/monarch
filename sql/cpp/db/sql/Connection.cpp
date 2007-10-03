/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/Connection.h"

using namespace db::sql;
using namespace db::rt;

Connection::Connection()
{
   mUrl = NULL;
}

Connection::~Connection()
{
   if(mUrl != NULL)
   {
      delete mUrl;
   }
}

SqlException* Connection::commit()
{
   return (SqlException*)Exception::setLast(
      new SqlException("Connection::commit() not supported!"));
}

SqlException* Connection::rollback()
{
   return (SqlException*)Exception::setLast(
      new SqlException("Connection::rollback() not supported!"));
}
