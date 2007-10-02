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
   mysql_stmt_fetch_column(getStatementHandle(), mBindings, column, 0);
   
   // FIXME: check exceptions, etc
   return NULL;
}

DatabaseException* MySqlRow::getInt64(int column, long long& i)
{
   mBindings[column].buffer_type = MYSQL_TYPE_LONGLONG;
   mBindings[column].buffer = (char*)&i;
   mBindings[column].buffer_length = 8;
   mBindings[column].length = &mBindings[column].buffer_length;
   mysql_stmt_fetch_column(getStatementHandle(), mBindings, column, 0);
   
   // FIXME: check exceptions, etc
   return NULL;
}
#include <iostream>
DatabaseException* MySqlRow::getText(int column, string& str)
{
   std::cout << "field count=" << mFieldCount << std::endl;
   std::cout << "line0" << std::endl;
   mBindings[column].buffer_type = MYSQL_TYPE_BLOB;
   std::cout << "line1" << std::endl;
   unsigned long length = mBindings[column].buffer_length;// + 1;
   std::cout << "now length: " << column << "=" << length << std::endl;
   std::cout << "line2" << std::endl;
   char temp[length + 1];
   std::cout << "line3" << std::endl;
   
   mBindings[column].buffer = temp;
   std::cout << "line4" << std::endl;
   mBindings[column].buffer_length = length;
   std::cout << "line5" << std::endl;
   mBindings[column].length = &length;
   std::cout << "line7" << std::endl;
   mysql_stmt_fetch_column(getStatementHandle(), mBindings, column, 0);
   
   std::cout << "line8" << std::endl;
   memset(temp + length, 0, 1);
   std::cout << "line9" << std::endl;
   str.assign(temp);
   
   // FIXME: check exceptions, etc
   return NULL;
}
