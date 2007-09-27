/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/database/Connection.h"

using namespace db::database;

Connection::Connection(const char *params)
{
   mDatabaseParams = new db::net::Url(params);
}

Connection::~Connection()
{
   delete mDatabaseParams;
}
