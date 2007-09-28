/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/database/sqlite3/Sqlite3RowIterator.h"
#include "db/database/sqlite3/Sqlite3Statement.h"

using namespace db::database;
using namespace db::database::sqlite3;

Sqlite3RowIterator::Sqlite3RowIterator(Sqlite3Statement* s) :
   RowIterator(s),
   mRow(s)
{
}

Sqlite3RowIterator::~Sqlite3RowIterator()
{
}

Row& Sqlite3RowIterator::next()
{
//   ret = sqlite3_step(((Sqlite3Statement*)mStatement)->mSqlite3Statement);
//
//   if(ret == SQLITE_ROW)
//   {
//      rval = true;
//   }
   
   
   
   return mRow;
}

bool Sqlite3RowIterator::hasNext()
{
   int ret;
   int rval = false;

   ret = sqlite3_step(((Sqlite3Statement*)mStatement)->mHandle);

   if(ret == SQLITE_ROW)
   {
      rval = true;
   }
   // FIXME else handle error

   return rval;
}
