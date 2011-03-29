/*
 * Copyright (c) 2009-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/mysql/MySqlDatabaseClient.h"

using namespace monarch::rt;
using namespace monarch::sql;
using namespace monarch::sql::mysql;

MySqlDatabaseClient::MySqlDatabaseClient()
{
}

MySqlDatabaseClient::~MySqlDatabaseClient()
{
}

SqlExecutableRef MySqlDatabaseClient::insertOrIgnore(
   const char* table, DynamicObject& row)
{
   return insertOrReplace("INSERT IGNORE", table, row);
}

SqlExecutableRef MySqlDatabaseClient::insertOnDuplicateKeyUpdate(
   const char* table, DynamicObject& row)
{
   SqlExecutableRef rval(NULL);

   // ensure the schema exists
   if(checkForSchema(table))
   {
      // get schema
      SchemaObject& schema = mSchemas[table];

      // create sql executable
      rval = new SqlExecutable();
      rval->write = true;

      // build parameters
      buildParams(schema, row, rval->params, table);

      // create starting clause
      rval->sql = "INSERT INTO ";
      rval->sql.append(schema["table"]->getString());

      // save sql string index after table
      rval->idxAfterTable = rval->sql.length();

      // append VALUES SQL
      appendValuesSql(rval->sql, rval->params);

      // append special update clause and SET sql
      rval->sql.append(" ON DUPLICATE KEY UPDATE");
      appendSetSql(rval->sql, rval->params);

      // clone parameters (need one set for INSERT one for UPDATE)
      DynamicObject updateParams = rval->params.clone();
      rval->params.merge(updateParams, true);
   }

   return rval;
}
