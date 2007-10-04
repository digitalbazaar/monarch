/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
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
      string msg;
      msg.append("Could not get column value, invalid column name!, name='");
      msg.append(name);
      msg.append(1, '\'');
      Exception::setLast(new SqlException(msg.c_str())); 
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

SqlException* MySqlRow::getType(unsigned int column, int& type)
{
   SqlException* rval = NULL;
   
   if(column >= mFieldCount)
   {
      char temp[100];
      sprintf(
         temp, "Could not get column type, invalid column index!,index=%i",
         column);
      rval = new SqlException(temp);
      Exception::setLast(rval);
   }
   else
   {
      type = mFields[column].type;
   }
   
   return rval;
}

SqlException* MySqlRow::getInt32(unsigned int column, int& i)
{
   SqlException* rval = NULL;
   
   mBindings[column].buffer_type = MYSQL_TYPE_LONG;
   mBindings[column].buffer = (char*)&i;
   mBindings[column].buffer_length = 4;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(getStatementHandle(), &mBindings[column], column, 0);
   
   // FIXME: check exceptions, etc
   return rval;
}

SqlException* MySqlRow::getInt64(unsigned int column, long long& i)
{
   SqlException* rval = NULL;
   
   mBindings[column].buffer_type = MYSQL_TYPE_LONGLONG;
   mBindings[column].buffer = (char*)&i;
   mBindings[column].buffer_length = 8;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(getStatementHandle(), &mBindings[column], column, 0);
   
   // FIXME: check exceptions, etc
   return rval;
}

SqlException* MySqlRow::getText(unsigned int column, string& str)
{
   SqlException* rval = NULL;
   
   mBindings[column].buffer_type = MYSQL_TYPE_BLOB;
   char temp[mBindings[column].buffer_length + 1];
   mBindings[column].buffer = temp;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(getStatementHandle(), &mBindings[column], column, 0);
   
   memset(temp + mBindings[column].buffer_length, 0, 1);
   str.assign(temp);
   
   // FIXME: check exceptions, etc
   return rval;
}

SqlException* MySqlRow::getType(const char* column, int& type)
{
   SqlException* rval = NULL;
   
   // get column index for name
   long long index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getType(index, type);
   }
   else
   {
      rval = (SqlException*)Exception::getLast();
   }
   
   return rval;
}

SqlException* MySqlRow::getInt32(const char* column, int& i)
{
   SqlException* rval = NULL;
   
   // get column index for name
   long long index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getInt32(index, i);
   }
   else
   {
      rval = (SqlException*)Exception::getLast();
   }
   
   return rval;
}

SqlException* MySqlRow::getInt64(const char* column, long long& i)
{
   SqlException* rval = NULL;
   
   // get column index for name
   long long index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getInt64(index, i);
   }
   else
   {
      rval = (SqlException*)Exception::getLast();
   }
   
   return rval;
}

SqlException* MySqlRow::getText(const char* column, std::string& str)
{
   SqlException* rval = NULL;
   
   // get column index for name
   long long index = getColumnIndex(column);
   if(index != -1)
   {
      rval = getText(index, str);
   }
   else
   {
      rval = (SqlException*)Exception::getLast();
   }
   
   return rval;
}
