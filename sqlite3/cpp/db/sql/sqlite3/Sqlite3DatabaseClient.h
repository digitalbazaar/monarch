/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_sql_sqlite3_Sqlite3DatabaseClient_H
#define db_sql_sqlite3_Sqlite3DatabaseClient_H

#include "db/sql/DatabaseClient.h"

namespace db
{
namespace sql
{
namespace sqlite3
{

/**
 * An Sqlite3DatabaseClient provides sqlite3 specific overloads for the
 * DatabaseClient class.
 * 
 * @author Dave Longley
 */
class Sqlite3DatabaseClient : public db::sql::DatabaseClient
{
public:
   /**
    * Creates a new Sqlite3DatabaseClient.
    */
   Sqlite3DatabaseClient();
   
   /**
    * Destructs this Sqlite3DatabaseClient.
    */
   virtual ~Sqlite3DatabaseClient();
   
   /**
    * Creates an SqlExecutable that will update a row in a table. All
    * applicable values in the given object will be updated in the given
    * table, according to its schema. If the given "where" object is not
    * NULL, its applicable members will define the WHERE clause of the
    * UPDATE SQL. An optional LIMIT amount may be specified.
    * 
    * This method is overloaded to use a sub-select query to apply any
    * limits that are specified due to a lack of support for UPDATE LIMITs
    * by default in sqlite3.
    * 
    * @param table the name of the table to UPDATE.
    * @param row the object with data to use in the update.
    * @param where the object with containing WHERE clause parameters.
    * @param limit 0 for no LIMIT, something positive to specify a LIMIT.
    * @param start the starting row for the LIMIT, defaults to 0.
    * 
    * @return the SqlExecutable if successful, NULL if an Exception occurred.
    */
   virtual SqlExecutableRef update(
      const char* table, db::rt::DynamicObject& row,
      db::rt::DynamicObject* where = NULL,
      uint64_t limit = 0, uint64_t start = 0);
};

} // end namespace sqlite3
} // end namespace sql
} // end namespace db
#endif
