/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/database/mysql/MySqlRow.h"
#include "db/database/mysql/MySqlStatement.h"
#include "db/database/mysql/MySqlConnection.h"

using namespace std;
using namespace db::database;
using namespace db::database::mysql;

MySqlRow::MySqlRow(MySqlStatement* s) : Row(s)
{
   mColumnCount = 0;
   mLengths = NULL;
}

MySqlRow::~MySqlRow()
{
}

MYSQL_STMT* MySqlRow::getStatementHandle()
{
   return ((MySqlStatement*)mStatement)->mHandle;
}

unsigned int MySqlRow::getColumn(const char* name)
{
   unsigned int rval = 0;
   
   for(unsigned int i = 0; i < mColumnCount; i++)
   {
      if(strcmp(mFields[i].name, name) == 0)
      {
         rval = i;
         break;
      }
   }
   
   return rval;
}

void MySqlRow::setData(
   MYSQL_ROW& row, unsigned int columns, unsigned long* lengths,
   MYSQL_FIELD* fields)
{
   mData = row;
   mColumnCount = columns;
   mLengths = lengths;
   mFields = fields;
}

DatabaseException* MySqlRow::getType(int column, int& type)
{
   // FIXME: check exceptions, etc
   type = mFields[column].type;
   return NULL;
}

DatabaseException* MySqlRow::getInt32(int column, int& i)
{
   MYSQL_BIND bind[1];
   bind[0].buffer = (char*)&i;
   bind[0].buffer_length = 4;
   bind[0].length = NULL;
   mysql_stmt_fetch_column(getStatementHandle(), bind, column, 0);
   
   // FIXME: check exceptions, etc
   return NULL;
}

DatabaseException* MySqlRow::getInt64(int column, long long& i)
{
   MYSQL_BIND bind[1];
   bind[0].buffer = (char*)&i;
   bind[0].buffer_length = 4;
   bind[0].length = NULL;
   mysql_stmt_fetch_column(getStatementHandle(), bind, column, 0);
   
   // FIXME: check exceptions, etc
   return NULL;
}

DatabaseException* MySqlRow::getText(int column, string& str)
{
   char temp[mLengths[column] + 1];
   
   unsigned long length;
   
   MYSQL_BIND bind[1];
   bind[0].buffer = temp;
   bind[0].buffer_length = mLengths[column] + 1;
   bind[0].length = &length;
   mysql_stmt_fetch_column(getStatementHandle(), bind, column, 0);
   
   memset(temp + mLengths[column], 0, 1);
   str.assign(temp);
   
   // FIXME: check exceptions, etc
   return NULL;
}
