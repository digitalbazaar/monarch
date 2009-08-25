/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/sql/mysql/MySqlRow.h"

#include "db/sql/mysql/MySqlStatement.h"
#include "db/sql/mysql/MySqlConnection.h"

using namespace std;
using namespace db::rt;
using namespace db::sql;
using namespace db::sql::mysql;

MySqlRow::MySqlRow(MySqlStatement* s) : Row(s)
{
   mFields = NULL;
   mFieldCount = 0;
   mBindings = NULL;
}

MySqlRow::~MySqlRow()
{
}

static inline MYSQL_STMT* getStatementHandle(Statement* s)
{
   return ((MySqlStatement*)s)->getHandle();
}

void MySqlRow::setFields(
   MYSQL_FIELD* fields, unsigned int count, MYSQL_BIND* bindings)
{
   mFields = fields;
   mFieldCount = count;
   mBindings = bindings;
}

bool MySqlRow::getType(unsigned int column, int& type)
{
   bool rval = false;

   if(column >= mFieldCount)
   {
      char temp[100];
      snprintf(temp, 100,
         "Could not get column type, invalid column index!,index=%i", column);
      ExceptionRef e = new SqlException(temp);
      Exception::set(e);
   }
   else
   {
      type = mFields[column].type;
      rval = true;
   }

   return rval;
}

bool MySqlRow::getInt32(unsigned int column, int32_t& i)
{
   mBindings[column].buffer_type = MYSQL_TYPE_LONG;
   mBindings[column].buffer = (char*)&i;
   mBindings[column].buffer_length = 4;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(
      getStatementHandle(mStatement), &mBindings[column], column, 0);

   // FIXME: check exceptions, etc
   return true;
}

bool MySqlRow::getUInt32(unsigned int column, uint32_t& i)
{
   mBindings[column].buffer_type = MYSQL_TYPE_LONG;
   mBindings[column].buffer = (char*)&i;
   mBindings[column].buffer_length = 4;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(
      getStatementHandle(mStatement), &mBindings[column], column, 0);

   // FIXME: check exceptions, etc
   return true;
}

bool MySqlRow::getInt64(unsigned int column, int64_t& i)
{
   mBindings[column].buffer_type = MYSQL_TYPE_LONGLONG;
   mBindings[column].buffer = (char*)&i;
   mBindings[column].buffer_length = 8;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(
      getStatementHandle(mStatement), &mBindings[column], column, 0);

   // FIXME: check exceptions, etc
   return true;
}

bool MySqlRow::getUInt64(unsigned int column, uint64_t& i)
{
   mBindings[column].buffer_type = MYSQL_TYPE_LONGLONG;
   mBindings[column].buffer = (char*)&i;
   mBindings[column].buffer_length = 8;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(
      getStatementHandle(mStatement), &mBindings[column], column, 0);

   // FIXME: check exceptions, etc
   return true;
}

bool MySqlRow::getText(unsigned int column, string& str)
{
   my_bool isNull;
   mBindings[column].buffer_type = MYSQL_TYPE_BLOB;
   char temp[mBindings[column].buffer_length + 1];
   mBindings[column].buffer = temp;
   mBindings[column].length = &mBindings[column].buffer_length;
   mBindings[column].is_null = &isNull;
   mysql_stmt_fetch_column(
      getStatementHandle(mStatement), &mBindings[column], column, 0);

   if(isNull)
   {
      str.erase();
   }
   else
   {
      temp[mBindings[column].buffer_length] = 0;
      str.assign(temp);
   }

   // FIXME: check exceptions, etc
   return true;
}

bool MySqlRow::getType(const char* column, int& type)
{
   bool rval = false;

   // get column index for name
   int64_t index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getType(index, type);
   }

   return rval;
}

bool MySqlRow::getInt32(const char* column, int32_t& i)
{
   bool rval = false;

   // get column index for name
   int64_t index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getInt32(index, i);
   }

   return rval;
}

bool MySqlRow::getUInt32(const char* column, uint32_t& i)
{
   bool rval = false;

   // get column index for name
   int64_t index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getUInt32(index, i);
   }

   return rval;
}

bool MySqlRow::getInt64(const char* column, int64_t& i)
{
   bool rval = false;

   // get column index for name
   int64_t index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getInt64(index, i);
   }

   return rval;
}

bool MySqlRow::getUInt64(const char* column, uint64_t& i)
{
   bool rval = false;

   // get column index for name
   int64_t index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getUInt64(index, i);
   }

   return rval;
}

bool MySqlRow::getText(const char* column, std::string& str)
{
   bool rval = false;

   // get column index for name
   int64_t index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getText(index, str);
   }

   return rval;
}

int64_t MySqlRow::getColumnIndex(const char* name)
{
   // use 64-bit signed int to cover all values + error (negative 1)
   int64_t rval = -1;

   for(unsigned int i = 0; i < mFieldCount; i++)
   {
      if(strcmp(name, mFields[i].name) == 0)
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
