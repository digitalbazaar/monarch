/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/database/sqlite3/Sqlite3Statement.h"
#include "db/database/sqlite3/Sqlite3Connection.h"
#include "db/database/sqlite3/Sqlite3RowIterator.h"

using namespace db::database;
using namespace db::database::sqlite3;

Sqlite3Statement::Sqlite3Statement(Sqlite3Connection *c, const char* sql) :
      Statement(c, sql)
{
   const char* tail;
   // FIXME: switch to sqlite3_prepare_v2 when appropriate
   sqlite3_prepare(c->mHandle, sql, -1, &mSqlite3Statement, &tail);
   mRowIterator = new Sqlite3RowIterator(this);
}

Sqlite3Statement::~Sqlite3Statement()
{
   sqlite3_finalize(mSqlite3Statement);
   delete mRowIterator;
}

void Sqlite3Statement::setInteger(int pos, int value)
{
   sqlite3_bind_int(mSqlite3Statement, pos, value);
}

void Sqlite3Statement::setText(int pos, const char* value)
{
   sqlite3_bind_text(mSqlite3Statement, pos, value, -1, SQLITE_STATIC);
   // FIXME STATIC vs ...
}

DatabaseException* Sqlite3Statement::execute()
{
   // FIXME:
   return NULL;
}

//RowIterator* Sqlite3Statement::executeQuery()
//{
//   return mRowIterator;
//}
//
//int Sqlite3Statement::executeUpdate()
//{
//   int ret;
//   int rval;
//
//   ret = sqlite3_step(mSqlite3Statement);
//
//   if(ret == SQLITE_DONE)
//   {
//      rval = sqlite3_changes(((Sqlite3Connection*)mConnection)->mHandle);
//   }
//   // FIXME else handle error
//
//   return rval;
//}

//int Sqlite3Statement::getErrorCode()
//{
//   return sqlite3_errcode(((Sqlite3Connection*)mConnection)->mHandle);
//}
//
//const char* Sqlite3Statement::getErrorMessage()
//{
//   return sqlite3_errmsg(((Sqlite3Connection*)mConnection)->mHandle);
//}
