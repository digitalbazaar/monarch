/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/Statement.h"

using namespace db::sql;

Statement::Statement(Connection* c, const char* sql)
{
   mConnection = c;
   
   mSql = new char[strlen(sql) + 1];
   strcpy(mSql, sql);
}

Statement::~Statement()
{
   delete [] mSql;
}

Connection* Statement::getConnection()
{
   return mConnection;
}
