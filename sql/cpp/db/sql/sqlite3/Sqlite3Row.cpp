/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/sqlite3/Sqlite3Row.h"
#include "db/sql/sqlite3/Sqlite3Statement.h"
#include "db/sql/sqlite3/Sqlite3Connection.h"

using namespace std;
using namespace db::sql;
using namespace db::sql::sqlite3;

Sqlite3Row::Sqlite3Row(Sqlite3Statement* s) : Row(s)
{
}

Sqlite3Row::~Sqlite3Row()
{
}

sqlite3_stmt* Sqlite3Row::getStatementHandle()
{
   return ((Sqlite3Statement*)mStatement)->mHandle;
}

SqlException* Sqlite3Row::getType(int column, int& type)
{
   // FIXME: check exceptions, etc
   type = sqlite3_column_type(getStatementHandle(), column);
   return NULL;
}

SqlException* Sqlite3Row::getInt32(int column, int& i)
{
   // FIXME: check exceptions, etc
   i = sqlite3_column_int(getStatementHandle(), column);
   return NULL;
}

SqlException* Sqlite3Row::getInt64(int column, long long& i)
{
   // FIXME: check exceptions, etc
   i = sqlite3_column_int64(getStatementHandle(), column);
   return NULL;
}

SqlException* Sqlite3Row::getText(int column, string& str)
{
   // FIXME: check exceptions, etc
   str.assign((const char*)sqlite3_column_text(getStatementHandle(), column));
   return NULL;
}
