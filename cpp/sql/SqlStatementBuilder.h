/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_SqlStatementBuilder_H
#define monarch_sql_SqlStatementBuilder_H

#include "monarch/sql/DatabaseClient.h"

#include <vector>

namespace monarch
{
namespace sql
{

/**
 * An SqlStatementBuilder provides a basic SQL StatementBuilder implementation
 * so that specific StatementBuilders that use SQL can reuse code by extending
 * this class.
 *
 * An SqlStatementBuilder is used to construct and execute SQL statements
 * in a way that abstracts away the actual SQL. This provides a way to write
 * more concise, reusable, and maintainable database client code.
 *
 * An SqlStatementBuilder makes use of a DatabaseClient to determine the
 * appropriate object-relational mappings between object members and database
 * columns.
 *
 * @author Dave Longley
 */
class SqlStatementBuilder : public StatementBuilder
{
protected:
   /**
    * The DatabaseClient to use.
    */
   DatabaseClient* mDatabaseClient;

   /**
    * A mapping of tables to aliases.
    */
   monarch::rt::DynamicObject mAliases;

   /**
    * A map of used aliases for detecting conflicts.
    */
   monarch::rt::DynamicObject mUsedAliases;

   /**
    * The counter for auto-assigned aliases.
    */
   int mAliasCounter;

   /**
    * The input objects.
    */
   monarch::rt::DynamicObject mObjects;

   /**
    * Stores a limit for the number of objects to return.
    */
   monarch::rt::DynamicObject mLimit;

   /**
    * A cache of the prepared statements.
    */
   typedef std::vector<Statement*> StatementCache;
   StatementCache mStatementCache;

   /**
    * Fetched results from a get statement.
    */
   monarch::rt::DynamicObject mResults;

public:
   /**
    * Creates a new SqlStatementBuilder.
    *
    * @param dbc the DatabaseClient to use.
    */
   SqlStatementBuilder(DatabaseClient* dbc);

   /**
    * Destructs this SqlStatementBuilder.
    */
   virtual ~SqlStatementBuilder();

   /**
    * Starts building a statement that will add an object.
    *
    * @param type the type of object.
    * @param obj the object to insert.
    *
    * @return a reference to this SqlStatementBuilder to permit chaining.
    */
   virtual StatementBuilder* add(
      const char* type, monarch::rt::DynamicObject& obj);

   /**
    * Starts building a statement that will update objects of the given type
    * by the given object.
    *
    * @param type the type of objects to update, as defined in an OR map.
    * @param obj the object with update values.
    * @param op an operational operator to use to set members to their
    *           values (defaults to '=').
    *
    * @return a reference to this SqlStatementBuilder to permit chaining.
    */
   virtual StatementBuilder* update(
      const char* type, monarch::rt::DynamicObject& obj,
      const char* op = "=");

   /**
    * Starts building a statement that will get objects of the given type.
    *
    * @param type the type of objects to get, as defined in an OR map.
    * @param obj an object with the members to be populated, NULL for all.
    *
    * @return a reference to this SqlStatementBuilder to permit chaining.
    */
   virtual StatementBuilder* get(
      const char* type, monarch::rt::DynamicObject* obj = NULL);

   /**
    * Places restrictions on the objects to get or update.
    *
    * If the statement being built will retrieve objects, then the given
    * params will provide conditionals to restrict the objects to retrieve.
    *
    * If the statement being built will update objects, then the given
    * params will provide conditionals to restrict the objects to update.
    *
    * @param type the type of object for the conditions.
    * @param conditions an object with members whose values will be used to
    *           create conditional restrictions in the current statement.
    * @param compareOp an operational operator to use to compare members to
    *           their values (defaults to '=').
    * @param boolOp the boolean operator to use to combine multiple member
    *           comparisons (defaults to "AND").
    *
    * @return a reference to this SqlStatementBuilder to permit chaining.
    */
   virtual StatementBuilder* where(
      const char* type,
      monarch::rt::DynamicObject& conditions,
      const char* compareOp = "=",
      const char* boolOp = "AND");

   /**
    * Limits the number of objects to update or get.
    *
    * @param count the number of objects to limit to.
    * @param start the starting row index.
    *
    * @return a reference to this SqlStatementBuilder to permit chaining.
    */
   virtual StatementBuilder* limit(int count, int start = 0);

   /**
    * Prepares and executes the built SQL statement(s).
    *
    * @param c the connection to use, NULL to obtain one from the database
    *           client's pool.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool execute(monarch::sql::Connection* c = NULL);

   /**
    * Fetches the result objects (following a get() call).
    *
    * @return an array of result objects.
    */
   virtual monarch::rt::DynamicObject fetch();

protected:
   /**
    * Blocks the use of any alias that matches a table name in the given
    * mapping.
    *
    * @param mapping the mapping to scan for table names.
    */
   virtual void blockAliases(monarch::rt::DynamicObject& mapping);

   /**
    * Assigns an alias to the given table, if one has not already been assigned.
    *
    * @param table the table to assign an alias to.
    *
    * @return the assigned alias.
    */
   virtual const char* assignAlias(const char* table);

   /**
    * Creates SQL statements and builds an array of parameters for each one.
    *
    * @param statements the object to hold the SQL statements and parameters.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool createSql(monarch::rt::DynamicObject& statements);

   /**
    * Creates SQL statements to add an object to a database.
    *
    * @param mapping the OR mapping.
    * @param statements the object to populate with statements and params.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool createAddSql(
      monarch::rt::DynamicObject& mapping,
      monarch::rt::DynamicObject& statements);

   /**
    * Creates SQL statements to update an object in a database.
    *
    * @param mapping the OR mapping.
    * @param statements the object to populate with statements and params.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool createUpdateSql(
      monarch::rt::DynamicObject& mapping,
      monarch::rt::DynamicObject& statements);

   /**
    * Creates SQL statements to get an object from a database.
    *
    * @param mapping the OR mapping.
    * @param statements the object to populate with statements and params.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool createGetSql(
      monarch::rt::DynamicObject& mapping,
      monarch::rt::DynamicObject& statements);

   /**
    * Gets the results from an SQL statement.
    *
    * @param s the current SQL statement.
    * @param idx the statement index in "statements".
    * @param statements the object with all statements, params, and row info.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool getResults(
      monarch::sql::Statement* s, int idx,
      monarch::rt::DynamicObject& statements);
};

} // end namespace sql
} // end namespace monarch
#endif
