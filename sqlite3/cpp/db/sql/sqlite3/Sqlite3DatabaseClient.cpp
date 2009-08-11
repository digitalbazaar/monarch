/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/sql/sqlite3/Sqlite3DatabaseClient.h"

using namespace db::rt;
using namespace db::sql;
using namespace db::sql::sqlite3;

Sqlite3DatabaseClient::Sqlite3DatabaseClient()
{
}

Sqlite3DatabaseClient::~Sqlite3DatabaseClient()
{
}

SqlExecutableRef Sqlite3DatabaseClient::update(
   const char* table, DynamicObject& row, DynamicObject* where,
   uint64_t limit, uint64_t start)
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
      
      // create starting clause
      rval->sql = "UPDATE ";
      rval->sql.append(schema["table"]->getString());
      
      // build SET parameters
      buildParams(schema, row, rval->params);
      
      // build WHERE parameters
      DynamicObject whereParams;
      whereParams->setType(Array);
      if(where != NULL)
      {
         rval->whereFilter = *where;
         buildParams(schema, *where, whereParams);
      }
      
      // append SET clause
      rval->sql.append(" SET");
      appendSetSql(rval->sql, rval->params);
      
      // append where clause
      appendWhereSql(rval->sql, whereParams, false);
      
      // Note: sqlite3 does not support LIMITs in UPDATEs by default, so
      // apply any limit via a special rowid sub-select query instead
      if(limit > 0)
      {
         if(where != NULL)
         {
            rval->sql.append(" AND ");
         }
         else
         {
            rval->sql.append(" WHERE ");
         }
         
         rval->sql.append("rowid IN (SELECT rowid FROM ");
         rval->sql.append(table);
         appendWhereSql(rval->sql, whereParams, false);
         
         // append LIMIT clause
         appendLimitSql(rval->sql, limit, start);
         
         // end sub-select
         rval->sql.append(")");
         
         // double where params
         DynamicObject whereParams2 = whereParams.clone();
         whereParams.merge(whereParams2, true);
      }
      
      // concatenate params
      rval->params.merge(whereParams, true);
   }
   
   return rval;
}
