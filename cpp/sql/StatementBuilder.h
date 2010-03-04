/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_StatementBuilder_H
#define monarch_sql_StatementBuilder_H

#include "monarch/sql/DatabaseClient.h"

namespace monarch
{
namespace sql
{

/**
 * A StatementBuilder is used to construct and execute SQL statements in a
 * way that abstracts away the actual SQL. This provides a way to write more
 * concise, reusable, and maintainable database client code.
 *
 * A StatementBuilder makes use of a DatabaseClient to determine the
 * appropriate object-relational mappings between object members and database
 * columns.
 *
 * A special sql() method is provided to allow any custom SQL to be added
 * to a statement while it is being constructed. This SQL will be added
 * literally to the current statement, therefore it will not be processed in
 * a way that will transform object members into column names.
 *
 * There are several methods provided to build SQL statements, ie select(),
 * insert(), join(). For those methods that take field names as parameters,
 * the field names may either be object member names or column names. A
 * preference will be given to searching for member names first when checking
 * the associated table's schema.
 *
 * @author Dave Longley
 */
class StatementBuilder
{
protected:
   /**
    * The DatabaseClient to use.
    */
   DatabaseClientRef mDatabaseClient;

   /**
    * The prepared SQL statement.
    */
   monarch::sql::Statement* mStatement;

   /**
    * Flag indicating whether or not the statement can modify the database.
    */
   bool mWrite;

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
    * The array of SQL clauses for building an SQL statement.
    */
   monarch::rt::DynamicObject mClauses;

public:
   /**
    * Creates a new StatementBuilder.
    *
    * @param dbc the DatabaseClient to use.
    */
   StatementBuilder(DatabaseClientRef& dbc);

   /**
    * Destructs this StatementBuilder.
    */
   virtual ~StatementBuilder();

   /**
    * Adds whatever custom SQL is desired to the current statement.
    *
    * @param sql the SQL to add to the current statement.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& sql(const char* sql);

   /**
    * Provides an array of parameters to use. This method will replace any
    * existing parameters.
    *
    * @param params an array of key-value pairs, where the key is a field name
    *           and the value is the value for the field (or an array of
    *           values).
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& setParams(monarch::rt::DynamicObject& params);

   /**
    * Creates an alias for a table. Aliases for tables will be created
    * automatically if using built-in methods like join(). But if custom
    * sql() needs to be written, then alias() is a useful method.
    *
    * Example:
    *
    * .select("mytable", "foo")
    * .alias("mytable", "t1")
    * .select("mytable2", "bar")
    * .sql("JOIN mytable2 t2 ON t2.foo=t1.foo")
    *
    * @param table the name of the table.
    * @param alias the alias for the table.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& alias(const char* table, const char* alias);

   /**
    * Creates an alias for a field.
    *
    * Example:
    *
    * .select("mytable", "foo")
    * .alias("mytable", "foo", "bar")
    *
    * Now the member or column "foo" will be returned as "bar" in the result.
    *
    * @param table the name of the associated table.
    * @param field the name of the field.
    * @param alias the alias for the field.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& alias(
      const char* table, const char* field, const char* alias);

   /**
    * Starts a SELECT clause or adds fields to a current one.
    *
    * @param table the name of the table to select the fields from.
    * @param fields a string containing the comma-delimited names of the fields
    *           to select, where the field names may be member names or column
    *           names.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& select(const char* table, const char* fields);

   /**
    * Indicates the fields to perform a JOIN on. The first table is the
    * table being joined, the second is the other table with a field to
    * join on. In other words, the SQL created will be similar to:
    *
    * JOIN t1 ON t1.f1=t2.f2
    *
    * @param t1 the name of the first table with a field to join on.
    * @param f1 the name of the first table's field to join on.
    * @param t2 the name of the second table with a field to join on.
    * @param f2 the name of the second table's field to join on.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& join(
      const char* t1, const char* f1, const char* t2, const char* f2);

   /**
    * Starts an INSERT clause or adds fields to an existing one.
    *
    * @param table the name of the table to insert into.
    * @param fields a map of members or column names to the associated data to
    *           insert.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& insert(
      const char* table, monarch::rt::DynamicObject& fields);

   /**
    * Starts an INSERT clause or adds fields to an existing one. This method
    * is used when the data for the insert will come from the database itself,
    * ie through a sub-SELECT.
    *
    * @param table the name of the table to insert into.
    * @param fields a comma-delimited list of fields that are to be inserted.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& insert(const char* table, const char* fields);

   /**
    * Starts an UPDATE clause or adds fields to an existing one.
    *
    * @param table the name of the table to update.
    * @param fields a map of members or column names to the associated data to
    *           update.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& update(
      const char* table, monarch::rt::DynamicObject& fields);

   /**
    * Starts an UPDATE clause or adds fields to an existing one. This method
    * is used when the data for the update will come from the database itself,
    * ie through a sub-SELECT.
    *
    * @param table the name of the table to update.
    * @param fields a comma-delimited list of fields that are to be updated.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& update(const char* table, const char* fields);

   /**
    * Starts a WHERE clause or adds fields to an existing one. The where
    * conditions will be concatenated together using the given logical operator,
    * which can be "AND" or "OR". The fields parameter contains key-value pairs.
    * The key is a member or column name. The value is either a value to
    * compare against using "=" or it is a map with a "value" parameter and
    * an "operator" parameter like "<", "<>", or "LIKE".
    *
    * Example:
    *
    * DynamicObject fields;
    * fields["foo"] = "bar";
    * fields["foo2"]["value"] = 10;
    * fields["foo2"]["operator"] = "<";
    *
    * .select("mytable", "foo,foo2")
    * .where("mytable", fields, "AND")
    *
    * Produces:
    * SELECT foo,foo2 FROM mytable WHERE (foo='bar' AND foo2<10)
    *
    * @param table the name of the table with the fields.
    * @param fields a map of members or column names to the associated data to
    *           compare against, where if the data is a map, it will contain
    *           a "value" and an "operator" to use.
    * @param logicalOp either "AND" or "OR", defaults to "AND".
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& where(
      const char* table, monarch::rt::DynamicObject& fields,
      const char* logicalOp = "AND");

   /**
    * Starts an ORDER BY clause or adds fields to an existing one.
    *
    * @param table the name of the table the fields belong to.
    * @param fields the comma-delimited fields to order by.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& orderBy(const char* table, const char* fields);

   /**
    * Adds a limit clause.
    *
    * @param start the starting limit.
    * @param num the number to limit.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& limit(uint64_t start, uint64_t num);

   /**
    * Starts a parenthetical group.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& group();

   /**
    * Stops a parenthetical group.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& ungroup();

   /**
    * Prepares and executes the built SQL statement. Once this has been called,
    * the statement cannot be modified.
    *
    * @param params optional parameters to use instead of those provided
    *           during statement construction (typically done to reuse a
    *           statement), NULL to use those provided during construction.
    * @param c the connection to use, NULL to obtain one from the database
    *           client's pool.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool execute(
      monarch::rt::DynamicObject* params = NULL,
      monarch::sql::Connection* c = NULL);

protected:
   /**
    * Creates the full SQL for the statement and builds an array of parameters
    * for it.
    *
    * @param sql the SQL string to populate.
    * @param params the parameters array to populate.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool createSql(std::string& sql, monarch::rt::DynamicObject& params);
};

} // end namespace sql
} // end namespace monarch
#endif
