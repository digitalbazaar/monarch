/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
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

MYSQL_STMT* MySqlRow::getStatementHandle()
{
   return ((MySqlStatement*)mStatement)->mHandle;
}

long long MySqlRow::getColumnIndex(const char* name)
{
   // use 64-bit signed int to cover all values + error (negative 1)
   long long rval = -1;
   
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
      int length = 100 + strlen(name);
      char msg[length];
      snprintf(msg, length,
         "Could not get column value, invalid column name!, name='%s'", name);
      ExceptionRef e = new SqlException(msg);
      Exception::setLast(e, false); 
   }
   
   return rval;
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
      Exception::setLast(e, false);
   }
   else
   {
      type = mFields[column].type;
      rval = true;
   }
   
   return rval;
}

bool MySqlRow::getInt32(unsigned int column, int& i)
{
   mBindings[column].buffer_type = MYSQL_TYPE_LONG;
   mBindings[column].buffer = (char*)&i;
   mBindings[column].buffer_length = 4;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(getStatementHandle(), &mBindings[column], column, 0);
   
   // FIXME: check exceptions, etc
   return true;
}

bool MySqlRow::getUInt32(unsigned int column, unsigned int& i)
{
   mBindings[column].buffer_type = MYSQL_TYPE_LONG;
   mBindings[column].buffer = (char*)&i;
   mBindings[column].buffer_length = 4;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(getStatementHandle(), &mBindings[column], column, 0);
   
   // FIXME: check exceptions, etc
   return true;
}

bool MySqlRow::getInt64(unsigned int column, long long& i)
{
   mBindings[column].buffer_type = MYSQL_TYPE_LONGLONG;
   mBindings[column].buffer = (char*)&i;
   mBindings[column].buffer_length = 8;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(getStatementHandle(), &mBindings[column], column, 0);
   
   // FIXME: check exceptions, etc
   return true;
}

bool MySqlRow::getUInt64(unsigned int column, unsigned long long& i)
{
   mBindings[column].buffer_type = MYSQL_TYPE_LONGLONG;
   mBindings[column].buffer = (char*)&i;
   mBindings[column].buffer_length = 8;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(getStatementHandle(), &mBindings[column], column, 0);
   
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
   mysql_stmt_fetch_column(getStatementHandle(), &mBindings[column], column, 0);
   
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
   long long index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getType(index, type);
   }
   
   return rval;
}

bool MySqlRow::getInt32(const char* column, int& i)
{
   bool rval = false;
   
   // get column index for name
   long long index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getInt32(index, i);
   }
   
   return rval;
}

bool MySqlRow::getUInt32(const char* column, unsigned int& i)
{
   bool rval = false;
   
   // get column index for name
   long long index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getUInt32(index, i);
   }
   
   return rval;
}

bool MySqlRow::getInt64(const char* column, long long& i)
{
   bool rval = false;
   
   // get column index for name
   long long index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getInt64(index, i);
   }
   
   return rval;
}

bool MySqlRow::getUInt64(const char* column, unsigned long long& i)
{
   bool rval = false;
   
   // get column index for name
   long long index = getColumnIndex(column);
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
   long long index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getText(index, str);
   }
   
   return rval;
}
