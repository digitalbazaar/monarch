/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_StatementBuilder_H
#define monarch_sql_StatementBuilder_H

#include "monarch/sql/DatabaseClient.h"

#include <vector>

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
    * The type of statement(s).
    */
   enum StatementType
   {
      Add,
      Update,
      Get
   } mStatementType;

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
    * The type for the input object.
    */
   std::string mObjectType;

   /**
    * The input objects.
    */
   monarch::rt::DynamicObject mObjects;

   /**
    * A cache of the prepared statements.
    */
   typedef std::vector<Statement*> StatementCache;
   StatementCache mStatementCache;

   /**
    * Stores a limit for the number of objects to return.
    */
   monarch::rt::DynamicObject mLimit;

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
    * Starts building a statement that will add an object.
    *
    * @param type the type of object.
    * @param obj the object to insert.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& add(
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
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& update(
      const char* type, monarch::rt::DynamicObject& obj,
      const char* op = "=");

   /**
    * Starts building a statement that will get objects of the given type.
    *
    * @param type the type of objects to get, as defined in an OR map.
    * @param obj an object with the members to be populated, NULL for all.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& get(
      const char* type, monarch::rt::DynamicObject* obj = NULL);

   /**
    * Adds more object members to be set to an update statement.
    *
    * @param members the object with update values.
    * @param op an operational operator to use to set members to their
    *           values (defaults to '=').
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& set(
      monarch::rt::DynamicObject& members, const char* op = "=");

   /**
    * Places restrictions on the objects to get or update.
    *
    * If the statement being built will retrieve objects, then the given
    * params will provide conditionals to restrict the objects to retrieve.
    *
    * If the statement being built will update objects, then the given
    * params will provide conditionals to restrict the objects to update.
    *
    * @param conditions an object with members whose values will be used to
    *           create conditional restrictions in the current statement.
    * @param compareOp an operational operator to use to compare members to
    *           their values (defaults to '=').
    * @param boolOp the boolean operator to use to combine multiple member
    *           comparisons (defaults to "AND").
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& where(
      monarch::rt::DynamicObject& conditions,
      const char* compareOp = "=",
      const char* boolOp = "AND");

   /**
    * Limits the number of objects to update or get.
    *
    * @param count the number of objects to limit to.
    * @param start the starting row index.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder& limit(int count, int start = 0);

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
    * Fetches the next object (following a get() call).
    *
    * @return the next object, NULL if there are no more to fetch.
    */
   virtual monarch::rt::DynamicObject fetch();

#if 0
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

#endif
protected:
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
};

} // end namespace sql
} // end namespace monarch
#endif
