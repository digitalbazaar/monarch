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

void MySqlRow::setFields(
   MYSQL_FIELD* fields, unsigned int count, MYSQL_BIND* bindings)
{
   mFields = fields;
   mFieldCount = count;
   mBindings = bindings;
}

DatabaseException* MySqlRow::getType(int column, int& type)
{
   // FIXME: check exceptions, etc
   type = mFields[column].type;
   return NULL;
}

DatabaseException* MySqlRow::getInt32(int column, int& i)
{
   mBindings[column].buffer_type = MYSQL_TYPE_LONG;
   mBindings[column].buffer = (char*)&i;
   mBindings[column].buffer_length = 4;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(getStatementHandle(), &mBindings[column], column, 0);
   
   // FIXME: check exceptions, etc
   return NULL;
}

DatabaseException* MySqlRow::getInt64(int column, long long& i)
{
   mBindings[column].buffer_type = MYSQL_TYPE_LONGLONG;
   mBindings[column].buffer = (char*)&i;
   mBindings[column].buffer_length = 8;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(getStatementHandle(), &mBindings[column], column, 0);
   
   // FIXME: check exceptions, etc
   return NULL;
}

DatabaseException* MySqlRow::getText(int column, string& str)
{
   mBindings[column].buffer_type = MYSQL_TYPE_BLOB;
   char temp[mBindings[column].buffer_length + 1];
   mBindings[column].buffer = temp;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(getStatementHandle(), &mBindings[column], column, 0);
   
   memset(temp + mBindings[column].buffer_length, 0, 1);
   str.assign(temp);
   
   // FIXME: check exceptions, etc
   return NULL;
}
