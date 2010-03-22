/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_StatementBuilder_H
#define monarch_sql_StatementBuilder_H

#include "monarch/rt/Collectable.h"
#include "monarch/sql/Connection.h"

namespace monarch
{
namespace sql
{

/**
 * A StatementBuilder is used to construct and execute database statements in a
 * way that abstracts away the actual database language (ie SQL) and interface.
 * This provides a way to write more concise, reusable, and maintainable
 * database client code.
 *
 * @author Dave Longley
 */
class StatementBuilder
{
protected:
   /**
    * The type of statement(s).
    */
   enum StatementType
   {
      Add,
      Update,
      Get
   } mStatementType;

public:
   /**
    * Creates a new StatementBuilder.
    */
   StatementBuilder() {};

   /**
    * Destructs this StatementBuilder.
    */
   virtual ~StatementBuilder() {};

   /**
    * Starts building a statement that will add an object.
    *
    * @param type the type of object.
    * @param obj the object to insert.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder* add(
      const char* type, monarch::rt::DynamicObject& obj) = 0;

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
   virtual StatementBuilder* update(
      const char* type, monarch::rt::DynamicObject& obj,
      const char* op = "=") = 0;

   /**
    * Starts building a statement that will get objects of the given type.
    *
    * @param type the type of objects to get, as defined in an OR map.
    * @param obj an object with the members to be populated, NULL for all.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder* get(
      const char* type, monarch::rt::DynamicObject* obj = NULL) = 0;

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
    *           comparisons (defaults to "AND", FIXME: use an enum).
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder* where(
      const char* type,
      monarch::rt::DynamicObject& conditions,
      const char* compareOp = "=",
      const char* boolOp = "AND") = 0;

   // FIXME: add method to request total number of matching objects

   /**
    * Limits the number of objects to update or get.
    *
    * @param count the number of objects to limit to.
    * @param start the starting row index.
    *
    * @return a reference to this StatementBuilder to permit chaining.
    */
   virtual StatementBuilder* limit(int count, int start = 0) = 0;

   /**
    * Prepares and executes the built database statement(s).
    *
    * @param c the connection to use, NULL to obtain one from the database
    *           client's pool.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool execute(monarch::sql::Connection* c = NULL) = 0;

   /**
    * Fetches the result objects, following a get() call. If the the call
    * was add() then each result object will contain an "ids" map with the
    * auto-increment ID values mapped to their member names according to the
    * OR mapping. Another field "changed" will contain the number of objects
    * changed by an add() or update() call.
    *
    * The result object may also contain a "tables" map with table names
    * mapped to changed rows for each individual table modified.
    *
    * @return an array of result objects.
    */
   virtual monarch::rt::DynamicObject fetch() = 0;

   // FIXME: add method for getting total number of matching objects
};

// type definition for a reference counted StatementBuilder
typedef monarch::rt::Collectable<StatementBuilder> StatementBuilderRef;

} // end namespace sql
} // end namespace monarch
#endif
