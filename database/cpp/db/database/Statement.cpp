/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/database/Statement.h"

using namespace db::database;

Statement::Statement(Connection* c, const char* sql)
{
   mConnection = c;
   mSql = sql;
}

Statement::~Statement()
{
}

Connection* Statement::getConnection()
{
   return mConnection;
}
