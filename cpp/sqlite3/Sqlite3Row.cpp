/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/sqlite3/Sqlite3Row.h"

#include "monarch/sql/sqlite3/Sqlite3Statement.h"
#include "monarch/sql/sqlite3/Sqlite3Connection.h"

using namespace std;
using namespace monarch::rt;
using namespace monarch::sql;
using namespace monarch::sql::sqlite3;

Sqlite3Row::Sqlite3Row(Sqlite3Statement* s) : Row(s)
{
   mColumnCount = -1;
}

Sqlite3Row::~Sqlite3Row()
{
}

static inline sqlite3_stmt* getStatementHandle(Statement* s)
{
   return ((Sqlite3Statement*)s)->getHandle();
}

bool Sqlite3Row::getType(unsigned int column, int& type)
{
   // FIXME: check exceptions, etc
   type = sqlite3_column_type(getStatementHandle(mStatement), column);
   return true;
}

bool Sqlite3Row::getInt32(unsigned int column, int32_t& i)
{
   // FIXME: check exceptions, etc
   i = sqlite3_column_int(getStatementHandle(mStatement), column);
   return true;
}

bool Sqlite3Row::getUInt32(unsigned int column, uint32_t& i)
{
   // FIXME: check exceptions, etc
   i = sqlite3_column_int(getStatementHandle(mStatement), column);
   return true;
}

bool Sqlite3Row::getInt64(unsigned int column, int64_t& i)
{
   // FIXME: check exceptions, etc
   i = sqlite3_column_int64(getStatementHandle(mStatement), column);
   return true;
}

bool Sqlite3Row::getUInt64(unsigned int column, uint64_t& i)
{
   // FIXME: check exceptions, etc
   i = sqlite3_column_int64(getStatementHandle(mStatement), column);
   return true;
}

bool Sqlite3Row::getText(unsigned int column, string& str)
{
   // FIXME: check exceptions, etc
   const char* text = (const char*)sqlite3_column_text(
      getStatementHandle(mStatement), column);
   int bytes = sqlite3_column_bytes(getStatementHandle(mStatement), column);
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

bool Sqlite3Row::getInt32(const char* column, int32_t& i)
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

bool Sqlite3Row::getUInt32(const char* column, uint32_t& i)
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

bool Sqlite3Row::getInt64(const char* column, int64_t& i)
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

bool Sqlite3Row::getUInt64(const char* column, uint64_t& i)
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

int Sqlite3Row::getColumnIndex(const char* name)
{
   int rval = -1;

   // get column count as appropriate
   if(mColumnCount == -1)
   {
      mColumnCount = sqlite3_column_count(getStatementHandle(mStatement));
   }

   for(int i = 0; i < mColumnCount; i++)
   {
      if(strcmp(name, sqlite3_column_name(
         getStatementHandle(mStatement), i)) == 0)
      {
         rval = i;
         break;
      }
   }

   if(rval == -1)
   {
      // set exception
      ExceptionRef e = new SqlException(
         "Could not get column value. Invalid column name.");
      e->getDetails()["name"] = name;
      Exception::set(e);
   }

   return rval;
}
