/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/Statement.h"

using namespace db::sql;

Statement::Statement(Connection* c, const char* sql)
{
   mConnection = c;
   mSql = strdup(sql);
}

Statement::~Statement()
{
   free(mSql);
}

const char* Statement::getSql()
{
   return mSql;
}

Connection* Statement::getConnection()
{
   return mConnection;
}
