/*
 * Copyright (c) 2009-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_DatabaseClient_H
#define monarch_sql_DatabaseClient_H

#include "monarch/sql/ConnectionPool.h"
#include "monarch/sql/StatementBuilder.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/validation/Validation.h"

namespace monarch
{
namespace sql
{

// forward declarations
class Row;

/**
 * A SchemaObject contains the schema for a database table. It is used
 * to create a table and as a mapping between objects that make up the
 * input and output of database functions and the actual table schema
 * in the database. The schema defines which column names map to which
 * attributes in an input/output object.
 *
 * SchemaObject: {} of
 *   "table": "tableName"
 *   "columns": [] of
 *     "column_name": {} of
 *       "type": "DATABASE COLUMN TYPE" (same as used in CREATE TABLE SQL)
 *       "memberName": "columnName" (member name as used in an object)
 *       "memberType": the expected member type
 *       "columnType": the data type for the column
 */
typedef monarch::rt::DynamicObject SchemaObject;

/**
 * An ObjRelMap provides a mapping between an object and a relational database.
 *
 * ObjRelMap: {} of
 *    "objectType": object-type
 *    "autoIncrement": {} of
 *       "table-name": "member-name"
 *    "members": {} of
 *       "member-name": {} of
 *          "group": "columns" or "fkeys"
 *          "table": the table for the object data
 *          "column": the column for the object data (or foreign key if an fkey)
 *          "columnType": the data type for the column (or fcolumn if an fkey)
 *          "memberType": the data type for the member
 *          (if mappingType is "fkey")
 *          "ftable": the table with a key to store in "table" that maps to
 *             the object data
 *          "fkey": the column in "ftable" with the key to store in "table"
 *          "fcolumn": the column in "ftable" with the object data
 *          "encode": an array of database transformation functions to encode
 *             data that is entering the database
 *          "decode": an array of database transformation functions to decode
 *             data that is coming from the database
 */
typedef monarch::rt::DynamicObject ObjRelMap;

/**
 * An SqlExecutable is an object that contains prepared statement SQL,
 * parameters for that SQL, and store other useful state information. It
 * can be generated and executed by a DatabaseClient. When executed, it will
 * run the generated SQL, setting all parameters, retrieving rows, and keeping
 * track of state information like the number of affected rows and any
 * auto-increment update IDs.
 */
struct SqlExecutable
{
   /**
    * Stores prepared statement SQL.
    */
   std::string sql;

   /**
    * Stores whether or not the SQL should modify the database.
    */
   bool write;

   /**
    * Stores an array of column name + column value parameters to be inserted
    * into a prepared statement.
    */
   monarch::rt::DynamicObject params;

   /**
    * Stores an array of column schemas that are used to retrieve column
    * data after doing an SQL SELECT.
    */
   monarch::rt::DynamicObject columnSchemas;

   /**
    * An SQL WHERE filter. This is a map with member name => member values
    * that the SQL will be filtered on.
    */
   monarch::rt::DynamicObject whereFilter;

   /**
    * Stores the result from a SELECT. This can either be one row or many.
    */
   monarch::rt::DynamicObject result;

   /**
    * Stores the number of affected rows after execution.
    */
   uint64_t rowsAffected;

   /**
    * Stores the number of rows retrieved.
    */
   uint64_t rowsRetrieved;

   /**
    * Stores the total number of rows found if requested.
    */
   uint64_t rowsFound;

   /**
    * Stores the last insert ID after execution.
    */
   uint64_t lastInsertRowId;

   /**
    * Stores whether or not the total number of rows found should be returned.
    */
   bool returnRowsFound;

   /**
    * The string position after the table. Useful for special inserting special
    * SQL like USE|FORCE INDEX.
    */
   std::string::size_type idxAfterTable;

   /**
    * Initializes an SqlExecutable.
    */
   SqlExecutable() :
      write(false),
      columnSchemas(NULL),
      whereFilter(NULL),
      result(NULL),
      rowsAffected(0),
      rowsRetrieved(0),
      rowsFound(0),
      lastInsertRowId(0),
      returnRowsFound(false),
      idxAfterTable(0)
   {
      params->setType(monarch::rt::Array);
   };
};

// type definition for a reference counted SqlExecutable
typedef monarch::rt::Collectable<SqlExecutable> SqlExecutableRef;

/**
 * A DatabaseClient provides a simple interface to a database. The interface
 * abstracts away SQL and the connection and fetching APIs from its user,
 * removing much of the verbosity required to do basic database interaction.
 *
 * The interface is largely object based/driven. Schema objects must be
 * provided to the DatabaseClient to initialize its use with various tables
 * in a database and its interaction with objects that may not store the
 * data in a given table using the same column names as its own attributes.
 *
 * For instance, an object may have an attribute of "fooId" but the column
 * name in a related table may be "foo_id".
 *
 * Methods that have a "where" paramter use a Map with multiple entries using
 * the following formats:
 * {
 *    "{name1}": value1, (simple equality)
 *    "{name2}": [value2a, value2b, ...], (multiple values for "IN" clause)
 *    "{name3}": {
 *       "op": operation, (">=", etc)
 *       "value": value3
 *    },
 *    ...
 * }
 *
 * Methods that have an "order" paramter use an Array with multiple entires
 * using the following formats:
 * [
 *    {"{name2}": DatabaseClient::ASC},
 *    {"{name3}": DatabaseClient::DESC},
 *    ...
 * ]
 * This can be easily constructed with:
 *    order->append()["name2"] = DatabaseClient::ASC;
 *    order->append()["name3"] = DatabaseClient::DESC;
 *
 * @author Dave Longley
 */
class DatabaseClient
{
public:
   /**
    * ORDER direction
    */
   enum OrderDirection
   {
      ASC, DESC
   };

protected:
   /**
    * True to enable debug logging.
    */
   bool mDebugLogging;

   /**
    * A database read connection pool.
    */
   ConnectionPoolRef mReadPool;

   /**
    * A database write connection pool.
    */
   ConnectionPoolRef mWritePool;

   /**
    * Stores all schema objects, accessible via their table name.
    */
   monarch::rt::DynamicObject mSchemas;

   /**
    * Stores the schema validator.
    */
   monarch::validation::ValidatorRef mSchemaValidator;

   /**
    * Stores all OR map objects, accessible via their object type.
    */
   monarch::rt::DynamicObject mOrMaps;

   /**
    * Stores the OR map validator.
    */
   monarch::validation::ValidatorRef mOrMapValidator;

public:
   /**
    * Creates a new DatabaseClient.
    */
   DatabaseClient();

   /**
    * Destructs this DatabaseClient.
    */
   virtual ~DatabaseClient();

   /**
    * Initializes this DatabaseClient.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool initialize();

   /**
    * Sets whether or not debug logging is enabled.
    *
    * @param enabled true to enable debug logging, false not to.
    */
   virtual void setDebugLogging(bool enabled);

   /**
    * Sets a connection pool to draw read connections from.
    *
    * @param pool the read connection pool to use.
    */
   virtual void setReadConnectionPool(ConnectionPoolRef& pool);

   /**
    * Sets a connection pool to draw write connections from.
    *
    * @param pool the write connection pool to use.
    */
   virtual void setWriteConnectionPool(ConnectionPoolRef& pool);

   /**
    * Gets a read connection from the read pool.
    *
    * @return a Connection or NULL if a connection could not be made.
    */
   virtual monarch::sql::Connection* getReadConnection();

   /**
    * Gets a write connection from the write pool.
    *
    * @return a Connection or NULL if a connection could not be made.
    */
   virtual monarch::sql::Connection* getWriteConnection();

   /**
    * Defines the schema for a table. This will not do CREATE TABLE, it
    * will only make interfacing with the given table possible via the
    * methods on this object. Call create() to attempt to create the table.
    *
    * @param schema the schema for a table.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool define(SchemaObject& schema);

   /**
    * Gets a schema for a table.
    *
    * @param table the table to get the schema for.
    *
    * @return the schema for the table or NULL if it does not exist.
    */
   virtual SchemaObject getSchema(const char* table);

   /**
    * Sets an object-relational (OR) mapping for an object type.
    *
    * @param orMap the OR mapping to set.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool setObjRelMap(ObjRelMap& orMap);

   /**
    * Gets an object-relational (OR) mapping for an object type.
    *
    * @param objType the object type to get the mapping for.
    *
    * @return the OR map for the given object type or NULL if it does not exist.
    */
   virtual ObjRelMap getObjRelMap(const char* objType);

   /**
    * Uses an Object-Relational (OR) mapping to produce a mapping for a
    * particular object instance.
    *
    * @param objType the type of object.
    * @param obj the object instance.
    * @param mapping the mapping object to populate.
    * @param userData a user-data object to include with every column, foreign
    *           key or sub-object entry.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool mapInstance(
      const char* objType,
      monarch::rt::DynamicObject& obj,
      monarch::rt::DynamicObject& mapping,
      monarch::rt::DynamicObject* userData = NULL);

   /**
    * Creates a StatementBuilder.
    *
    * @return the created StatementBuilder.
    */
   virtual StatementBuilderRef createStatementBuilder();

   /**
    * Creates a table via CREATE TABLE. The schema for the table must have
    * been previously set with define.
    *
    * @param table the name of the table to create.
    * @param ignoreIfExists true to ignore any errors if the table already
    *           exists, false to report them.
    * @param c the connection to use, NULL to obtain one from the pool.
    *
    * @return the SqlExecutable if successful, NULL if an Exception occurred.
    */
   virtual bool create(
      const char* table, bool ignoreIfExists, Connection* c = NULL);

   /**
    * Drops a table via DROP TABLE.
    *
    * @param table the name of the table to delete.
    * @param ignoreIfNotExists true to ignore any errors if the table doesn't
    *           exist, false to report them.
    * @param c the connection to use, NULL to obtain one from the pool.
    *
    * @return the SqlExecutable if successful, NULL if an Exception occurred.
    */
   virtual bool drop(
      const char* table, bool ignoreIfNotExists, Connection* c = NULL);

   /**
    * Creates an SqlExecutable that will insert a row into a table. All
    * applicable values in the given object will be inserted into the given
    * table, according to its schema.
    *
    * @param table the name of the table to INSERT INTO.
    * @param row the object with data to insert as a row.
    *
    * @return the SqlExecutable if successful, NULL if an Exception occurred.
    */
   virtual SqlExecutableRef insert(
      const char* table, monarch::rt::DynamicObject& row);

   /**
    * Creates an SqlExecutable that will insert a row into a table. All
    * applicable values in the given object will be inserted into the given
    * table, according to its schema. If a duplicate key error occurs, it
    * will be ignored and no insert will occur. This can be checked by
    * inspecting the rowsAffected property on the sql executable.
    *
    * @param table the name of the table to INSERT OR IGNORE INTO.
    * @param row the object with data to insert as a row.
    *
    * @return the SqlExecutable if successful, NULL if an Exception occurred.
    */
   virtual SqlExecutableRef insertOrIgnore(
      const char* table, monarch::rt::DynamicObject& row);

   /**
    * Creates an SqlExecutable that will replace a row in a table. All
    * applicable values in the given object will be inserted into the given
    * table, according to its schema.
    *
    * @param table the name of the table to REPLACE INTO.
    * @param row the object with data to insert as a row.
    *
    * @return the SqlExecutable if successful, NULL if an Exception occurred.
    */
   virtual SqlExecutableRef replace(
      const char* table, monarch::rt::DynamicObject& row);

   /**
    * Creates an SqlExecutable that will insert a row into a table and if
    * a duplicate key is found, it will update that row. The default
    * implementation for this method will call replace(). Extending classes
    * may call more optimized database-specific SQL.
    *
    * @param table the name of the table to insert into/update.
    * @param row the object with data to insert/update as a row.
    *
    * @return the SqlExecutable if successful, NULL if an Exception occurred.
    */
   virtual SqlExecutableRef insertOnDuplicateKeyUpdate(
      const char* table, monarch::rt::DynamicObject& row);

   /**
    * Creates an SqlExecutable that will update a row in a table. All
    * applicable values in the given object will be updated in the given
    * table, according to its schema. If the given "where" object is not
    * NULL, its applicable members will define the WHERE clause of the
    * UPDATE SQL. An optional LIMIT amount may be specified.
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
      const char* table, monarch::rt::DynamicObject& row,
      monarch::rt::DynamicObject* where = NULL,
      uint64_t limit = 0, uint64_t start = 0);

   /**
    * Creates an SqlExecutable that will select column values from the
    * specified table. The first row will be returned in the SqlExecutable's
    * result property as a map. This property can be set before executing.
    *
    * @param table the name of the table to select from.
    * @param where an object that specifies some specific column values to
    *           filter on, NULL to include no WHERE clause.
    * @param members a specific map of member names to include, NULL to
    *           include all members.
    * @param order an array that specifies the order of results or NULL for
    *           default ordering.
    *
    * @return the SqlExecutable if successful, NULL if an Exception occurred.
    */
   virtual SqlExecutableRef selectOne(
      const char* table,
      monarch::rt::DynamicObject* where = NULL,
      monarch::rt::DynamicObject* members = NULL,
      monarch::rt::DynamicObject* order = NULL);

   /**
    * Creates an SqlExecutable that will select column values from the
    * specified table. An optional LIMIT amount may be specified. The row
    * results will be stored in the SqlExecutable's result property as an array.
    * This property can be set before executing.
    *
    * @param table the name of the table to SELECT FROM.
    * @param where an object that specifies some specific column values to
    *           filter on, NULL to include no WHERE clause.
    * @param members a specific map of member names to include, NULL to
    *           include all members.
    * @param limit 0 for no LIMIT, something positive to specify a LIMIT.
    * @param start the starting row for the LIMIT, defaults to 0.
    * @param order an array that specifies the order of results or NULL for
    *           default ordering.
    *
    * @return the SqlExecutable if successful, NULL if an Exception occurred.
    */
   virtual SqlExecutableRef select(
      const char* table,
      monarch::rt::DynamicObject* where = NULL,
      monarch::rt::DynamicObject* members = NULL,
      uint64_t limit = 0, uint64_t start = 0,
      monarch::rt::DynamicObject* order = NULL);

   /**
    * Creates an SqlExecutable that deletes from a table. If the given
    * "where" object is not NULL, its applicable members will define the
    * WHERE clause of the UPDATE SQL. An optional LIMIT may be specified.
    *
    * @param table the name of the table to DELETE FROM.
    * @param where the object with containing WHERE clause parameters.
    * @param limit 0 for no LIMIT, something positive to specify a LIMIT.
    * @param start the starting row for the LIMIT, defaults to 0.
    *
    * @return the SqlExecutable if successful, NULL if an Exception occurred.
    */
   virtual SqlExecutableRef remove(
      const char* table, monarch::rt::DynamicObject* where,
      uint64_t limit = 0, uint64_t start = 0);

   /**
    * Begins a database transaction.
    *
    * @param c the connection to use.
    */
   virtual bool begin(Connection* c);

   /**
    * Ends a transaction either with a COMMIT or a ROLLBACK. Which is used
    * is specified by the given boolean parameter.
    *
    * @param c the connection to use.
    * @param commit true to commit the transaction, false to roll it back.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool end(Connection* c, bool commit);

   /**
    * Executes the passed SqlExecutable. Its SQL statement will be prepared,
    * its parameters set, and then executed. If a row object was provided, then
    * a single row will be retrieved. If a rows object was provided, then
    * X row(s) will be retrieved.
    *
    * @param se the SqlExecutable to execute.
    * @param c the connection to use, NULL to obtain one from the pool.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool execute(SqlExecutableRef& se, Connection* c = NULL);

   /**
    * Checks to make sure a schema for the given table exists.
    *
    * @param table the table to check for a schema for.
    *
    * @return true if a schema exists, false if not (with Exception set).
    */
   virtual bool checkForSchema(const char* table);

   /**
    * Converts a map of member-named values into an array of parameters. Each
    * member name that is recognized in the passed input object will be
    * converted into a parameter, that includes its associated table column
    * name, and it will be appended to the passed parameters array.
    *
    * @param schema the table schema to use.
    * @param members the input map of member-named values.
    * @param params the parameters array to append to.
    * @param tableAlias a table alias to use.
    */
   virtual void buildParams(
      SchemaObject& schema,
      monarch::rt::DynamicObject& members,
      monarch::rt::DynamicObject& params,
      const char* tableAlias = NULL);

   /**
    * Builds an array of column schema information for columns based on the
    * given maps of member-named values. The resulting column schemas array
    * will consist of a collection of columns that do not appear the in the
    * given excludeMembers object but do exist in the given includeMembers
    * object. If the includeMembers object is NULL then any members that
    * exist that are not in the excludeMembers object will be included. If
    * the excludeMembers object is NULL then it will be ignored entirely.
    *
    * Each entry in the resulting array is for a single column and contains
    * a reference to that column's schema information that includes column
    * name, type, and associated member name and type. This can be used
    * to generate the columns for a SELECT statement and to pull data out
    * of the rows that were selected and put it into an object.
    *
    * @param schema the table schema to use.
    * @param excludeMembers the input map of member-named values to exclude.
    * @param includeMembers the input map of member-named values to include.
    * @param columnSchemas the column schemas array to populate.
    * @param tableAlias a table alias to use.
    */
   virtual void buildColumnSchemas(
      SchemaObject& schema,
      monarch::rt::DynamicObject* excludeMembers,
      monarch::rt::DynamicObject* includeMembers,
      monarch::rt::DynamicObject& columnSchemas,
      const char* tableAlias = NULL);

   /**
    * Appends the SQL " (col1,col2,...) VALUES (val1,val2,...)" to an SQL
    * statement.
    *
    * @param sql the SQL string to append to.
    * @param params the list of parameters to generate the SQL from.
    */
   virtual void appendValuesSql(
      std::string& sql, monarch::rt::DynamicObject& params);

   /**
    * Appends the SQL " col1,col2,..." to an SQL statement.
    *
    * @param sql the SQL string to append to.
    * @param columnSchemas the column schemas array to generate the SQL from.
    */
   virtual void appendColumnNames(
      std::string& sql, monarch::rt::DynamicObject& columnSchemas);

   /**
    * Appends the SQL " WHERE col1=? AND col2=? ..." to an SQL statement.
    *
    * @param sql the SQL string to append to.
    * @param params the list of parameters to generate the SQL from.
    * @param useTableAlias true to use a table alias, false not to.
    */
   virtual void appendWhereSql(
      std::string& sql, monarch::rt::DynamicObject& params, bool useTableAlias);

   /**
    * Appends the SQL " ORDER BY col1 ASC|DESC, ..." to an SQL statement.
    *
    * @param sql the SQL string to append to.
    * @param params the list of parameters to generate the SQL from.
    * @param useTableAlias true to use a table alias, false not to.
    */
   virtual void appendOrderSql(
      std::string& sql, monarch::rt::DynamicObject& params, bool useTableAlias);

   /**
    * Appends the SQL " LIMIT <start>,<limit>" to an SQL statement.
    *
    * @param sql the SQL string to append to.
    * @param limit 0 for no LIMIT, something positive to specify a LIMIT.
    * @param start the starting row for the LIMIT, defaults to 0.
    */
   virtual void appendLimitSql(
      std::string& sql, uint64_t limit, uint64_t start);

   /**
    * Appends the SQL " col1=?,col2=? ..." to an SQL statement. Note that
    * " SET" is not prepended so that this function can be reused where
    * no " SET" should be provided.
    *
    * @param sql the SQL string to append to.
    * @param params the list of parameters to generate the SQL from.
    */
   virtual void appendSetSql(
      std::string& sql, monarch::rt::DynamicObject& params);

   /**
    * Sets the parameters for a statement.
    *
    * @param s the statement.
    * @param params the parameters.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool setParams(Statement* s, monarch::rt::DynamicObject& params);

   /**
    * Gets row data from a row returned from a statement.
    *
    * @param columnSchemas the column schemas associated with the row.
    * @param r the SQL row returned from a statement.
    * @param row the row object to put the SQL row data into.
    *
    * @return true if successful, false if an Exception occured.
    */
   virtual bool getRowData(
      monarch::rt::DynamicObject& columnSchemas,
      Row* r, monarch::rt::DynamicObject& row);

   /**
    * Creates SELECT SQL text and the associated parameters and column
    * schemas arrays.
    *
    * @param schema the table schema to use.
    * @param where the WHERE filter.
    * @param order the ORDER specification.
    * @param members a specific map of member names to include, NULL to
    *           include all members not in the WHERE.
    * @param limit 0 for no LIMIT, something positive to specify a LIMIT.
    * @param start the starting row for the LIMIT.
    * @param params the parameters to populate.
    * @param columnSchemas the column schemas to populate.
    * @param tableAlias a table alias to use.
    * @param se the SqlExecutableRef to update with SQL string indexes, NULL
    *           for none.
    *
    * @return the SQL SELECT text.
    */
   std::string createSelectSql(
      SchemaObject& schema,
      monarch::rt::DynamicObject* where,
      monarch::rt::DynamicObject* members,
      monarch::rt::DynamicObject* order,
      uint64_t limit,
      uint64_t start,
      monarch::rt::DynamicObject& params,
      monarch::rt::DynamicObject& columnSchemas,
      const char* tableAlias,
      SqlExecutableRef* se = NULL);

   /**
    * Appends a column to the given table schema.
    *
    * @param schema the schema to append to.
    * @param name the name of the column.
    * @param type the type of the column.
    * @param memberName the associated object member name.
    * @param memberType the associated object member type.
    */
   static void addSchemaColumn(
      SchemaObject& schema,
      const char* name, const char* type,
      const char* memberName, monarch::rt::DynamicObjectType memberType);

   /**
    * Appends a column to the given table schema.
    *
    * @param schema the schema to append to.
    * @param name the name of the column.
    * @param type the database type of the column.
    * @param memberName the associated object member name.
    * @param memberType the associated object member type.
    * @param columnType the object type for the column, if different from
    *           memberType.
    */
   static void addSchemaColumn(
      SchemaObject& schema,
      const char* name, const char* type,
      const char* memberName,
      monarch::rt::DynamicObjectType memberType,
      monarch::rt::DynamicObjectType columnType);

protected:
   /**
    * Creates an SqlExecutable that will insert or replace a row into a table.
    * All applicable values in the given object will be inserted into the given
    * table, according to its schema.
    *
    * @param cmd the command (INSERT or REPLACE).
    * @param table the name of the table to INSERT/REPLACE INTO.
    * @param row the object with data to insert as a row.
    *
    * @return the SqlExecutable if successful, NULL if an Exception occurred.
    */
   virtual SqlExecutableRef insertOrReplace(
      const char* cmd, const char* table, monarch::rt::DynamicObject& row);
};

// type definition for a reference counted DatabaseClient
typedef monarch::rt::Collectable<DatabaseClient> DatabaseClientRef;

} // end namespace sql
} // end namespace monarch
#endif
