/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
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
   mColumnCount = -1;
}

Sqlite3Row::~Sqlite3Row()
{
}

sqlite3_stmt* Sqlite3Row::getStatementHandle()
{
   return ((Sqlite3Statement*)mStatement)->mHandle;
}

int Sqlite3Row::getColumnIndex(const char* name)
{
   int rval = -1;
   
   // get column count as appropriate
   if(mColumnCount == -1)
   {
      mColumnCount = sqlite3_column_count(getStatementHandle());
   }
   
   for(int i = 0; i < mColumnCount; i++)
   {
      if(strcmp(name, sqlite3_column_name(getStatementHandle(), i)) == 0)
      {
         rval = i;
         break;
      }
   }
   
   if(rval == -1)
   {
      // set exception
      int length = strlen(name) + 60; 
      char temp[length];
      snprintf(temp, length,
         "Could not get column value, invalid column name!, name='%s'", name);
      ExceptionRef e = new SqlException(temp);
      Exception::setLast(e);
   }
   
   return rval;
}

bool Sqlite3Row::getType(unsigned int column, int& type)
{
   // FIXME: check exceptions, etc
   type = sqlite3_column_type(getStatementHandle(), column);
   return true;
}

bool Sqlite3Row::getInt32(unsigned int column, int& i)
{
   // FIXME: check exceptions, etc
   i = sqlite3_column_int(getStatementHandle(), column);
   return true;
}

bool Sqlite3Row::getUInt32(unsigned int column, unsigned int& i)
{
   // FIXME: check exceptions, etc
   i = sqlite3_column_int(getStatementHandle(), column);
   return true;
}

bool Sqlite3Row::getInt64(unsigned int column, long long& i)
{
   // FIXME: check exceptions, etc
   i = sqlite3_column_int64(getStatementHandle(), column);
   return true;
}

bool Sqlite3Row::getUInt64(unsigned int column, unsigned long long& i)
{
   // FIXME: check exceptions, etc
   i = sqlite3_column_int64(getStatementHandle(), column);
   return true;
}

bool Sqlite3Row::getText(unsigned int column, string& str)
{
   // FIXME: check exceptions, etc
   const char* text = (const char*)sqlite3_column_text(
      getStatementHandle(), column);
   int bytes = sqlite3_column_bytes(getStatementHandle(), column);
   str.assign(text, bytes);
   return true;
}

bool Sqlite3Row::getType(const char* column, int& type)
{
   bool rval = false;
   
   // get column index for name
   int index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getType(index, type);
   }
   
   return rval;
}

bool Sqlite3Row::getInt32(const char* column, int& i)
{
   bool rval = false;
   
   // get column index for name
   int index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getInt32(index, i);
   }
   
   return rval;
}

bool Sqlite3Row::getUInt32(const char* column, unsigned int& i)
{
   bool rval = false;
   
   // get column index for name
   int index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getUInt32(index, i);
   }
   
   return rval;
}

bool Sqlite3Row::getInt64(const char* column, long long& i)
{
   bool rval = false;
   
   // get column index for name
   int index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getInt64(index, i);
   }
   
   return rval;
}

bool Sqlite3Row::getUInt64(const char* column, unsigned long long& i)
{
   bool rval = false;
   
   // get column index for name
   int index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getUInt64(index, i);
   }
   
   return rval;
}

bool Sqlite3Row::getText(const char* column, std::string& str)
{
   bool rval = false;
   
   // get column index for name
   int index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getText(index, str);
   }
   
   return rval;
}
