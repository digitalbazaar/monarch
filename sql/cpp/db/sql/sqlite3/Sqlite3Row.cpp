/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/sqlite3/Sqlite3Row.h"
#include "db/sql/sqlite3/Sqlite3Statement.h"
#include "db/sql/sqlite3/Sqlite3Connection.h"

using namespace std;
using namespace db::rt;
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

SqlException* Sqlite3Row::getType(unsigned int column, int& type)
{
   SqlException* rval = NULL;
   
   // FIXME: check exceptions, etc
   type = sqlite3_column_type(getStatementHandle(), column);
   return rval;
}

SqlException* Sqlite3Row::getInt32(unsigned int column, int& i)
{
   SqlException* rval = NULL;
   
   // FIXME: check exceptions, etc
   i = sqlite3_column_int(getStatementHandle(), column);
   return rval;
}

SqlException* Sqlite3Row::getInt64(unsigned int column, long long& i)
{
   SqlException* rval = NULL;
   
   // FIXME: check exceptions, etc
   i = sqlite3_column_int64(getStatementHandle(), column);
   return rval;
}

SqlException* Sqlite3Row::getText(unsigned int column, string& str)
{
   SqlException* rval = NULL;
   
   // FIXME: check exceptions, etc
   str.assign((const char*)sqlite3_column_text(getStatementHandle(), column));
   return rval;
}

SqlException* Sqlite3Row::getType(const char* column, int& type)
{
   SqlException* rval = 
     new SqlException("Sqlite3 named column support not implemented!");
   Exception::setLast(rval);
   
   // FIXME: implement me
   
   return rval;
}

SqlException* Sqlite3Row::getInt32(const char* column, int& i)
{
   SqlException* rval = 
     new SqlException("Sqlite3 named column support not implemented!");
   Exception::setLast(rval);
   
   // FIXME: implement me
   
   return rval;
}

SqlException* Sqlite3Row::getInt64(const char* column, long long& i)
{
   SqlException* rval = 
     new SqlException("Sqlite3 named column support not implemented!");
   Exception::setLast(rval);
   
   // FIXME: implement me
   
   return rval;
}

SqlException* Sqlite3Row::getText(const char* column, std::string& str)
{
   SqlException* rval = 
     new SqlException("Sqlite3 named column support not implemented!");
   Exception::setLast(rval);
   
   // FIXME: implement me
   
   return rval;
}
