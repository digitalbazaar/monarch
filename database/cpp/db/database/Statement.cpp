/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/database/Statement.h"

using namespace db::database;

Statement::Statement(Connection* c, const char* sql)
{
   mConnection = c;
   
   mSql = new char[strlen(sql) + 1];
   strcpy(mSql, sql);
   
   mRowsChanged = 0;
}

Statement::~Statement()
{
   delete [] mSql;
}

void Statement::setRowsChanged(int count)
{
   mRowsChanged = count;
}

Connection* Statement::getConnection()
{
   return mConnection;
}

int Statement::getRowsChanged()
{
   return mRowsChanged;
}
