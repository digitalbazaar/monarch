/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/database/Connection.h"

using namespace db::database;
using namespace db::rt;

Connection::Connection(const char *url) :
   mUrl(url)
{
}

Connection::~Connection()
{
}

DatabaseException* Connection::commit()
{
   return (DatabaseException*)Exception::setLast(
      new DatabaseException("Connection::commit() not supported!"));
}

DatabaseException* Connection::rollback()
{
   return (DatabaseException*)Exception::setLast(
      new DatabaseException("Connection::rollback() not supported!"));
}
