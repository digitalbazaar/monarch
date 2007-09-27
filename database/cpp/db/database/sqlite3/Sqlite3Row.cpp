/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/database/sqlite3/Sqlite3Row.h"
#include "db/database/sqlite3/Sqlite3Statement.h"
#include "db/database/sqlite3/Sqlite3Connection.h"

using namespace db::database;
using namespace db::database::sqlite3;

Sqlite3Row::Sqlite3Row(Sqlite3Statement *s) :
   Row(s)
{
}

Sqlite3Row::~Sqlite3Row()
{
}

sqlite3_stmt* Sqlite3Row::getSqlite3Statement()
{
   return ((Sqlite3Statement*)mStatement)->mSqlite3Statement;
}

int Sqlite3Row::getType(int col)
{
   return sqlite3_column_type(getSqlite3Statement(), col);
}

int Sqlite3Row::getInteger(int col)
{
   return sqlite3_column_int(getSqlite3Statement(), col);
}

const char* Sqlite3Row::getText(int col)
{
   return (const char *)sqlite3_column_text(getSqlite3Statement(), col);
}
