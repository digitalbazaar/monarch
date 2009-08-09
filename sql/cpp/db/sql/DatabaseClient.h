/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_sql_DatabaseClient_H
#define db_sql_DatabaseClient_H

#include "db/sql/ConnectionPool.h"
#include "db/rt/DynamicObject.h"
#include "db/validation/Validation.h"

namespace db
{
namespace sql
{

// forward declare row
class Row;

/**
 * A SchemaObject contains the schema for a database table. It is used
 * to create a table and as a mapping between objects that make up the
 * input and output of database functions and the actual table schema
 * in the database. The schema defines which column names map to which
 * attributes in an input/output object.
 * 
 * SchemaObject: {} of
 *   "table": "tableName",
 *   "columns": [] of
 *     "column_name": {} of
 *       "type": "DATABASE COLUMN TYPE" (same as used in CREATE TABLE SQL),
 *       "memberName": "columnName" (member name as used in an object)
 */
typedef db::rt::DynamicObject SchemaObject;

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
 * @author Dave Longley
 */
class DatabaseClient
{
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
   db::rt::DynamicObject mSchemas;
   
   /**
    * Stores the schema validator.
    */
   db::validation::ValidatorRef mSchemaValidator;
   
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
   virtual db::sql::Connection* getReadConnection();
   
   /**
    * Gets a write connection from the write pool.
    * 
    * @return a Connection or NULL if a connection could not be made.
    */
   virtual db::sql::Connection* getWriteConnection();
   
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
    * Creates a table via CREATE TABLE. The schema for the table must have
    * been previously set with defineTable.
    * 
    * @param table the name of the table to create.
    * @param ignoreIfExists true to ignore any errors if the table already
    *           exists, false to report them.
    * @param c the connection to use, NULL to obtain one from the pool. 
    * 
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool create(
      const char* table, bool ignoreIfExists, Connection* c = NULL);
   
   /**
    * Inserts a row into a table. All applicable values in the given object
    * will be inserted into the given table, according to its schema.
    * 
    * @param table the name of the table to INSERT INTO.
    * @param row the object with data to insert as a row.
    * @param c the connection to use, NULL to obtain one from the pool.
    * 
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool insert(
      const char* table, db::rt::DynamicObject& row, Connection* c = NULL);
   
   /**
    * Replaces a row in a table. All applicable values in the given object
    * will be inserted into the given table, according to its schema.
    * 
    * @param table the name of the table to REPLACE INTO.
    * @param row the object with data to insert as a row.
    * @param c the connection to use, NULL to obtain one from the pool.
    * 
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool replace(
      const char* table, db::rt::DynamicObject& row, Connection* c = NULL);
   
   /**
    * Inserts or updates a row into a table. All applicable values in the given
    * object will be inserted into the given table, according to its schema.
    * 
    * The default implementation will call replace(). Extending implementations
    * may use more optimized SQL like INSERT INTO ... ON DUPLICATE KEY UPDATE.
    * 
    * @param table the name of the table to insert/update.
    * @param row the object with data to insert as a row.
    * @param c the connection to use, NULL to obtain one from the pool.
    * 
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool insertOrUpdate(
      const char* table, db::rt::DynamicObject& row, Connection* c = NULL);
   
   /**
    * Updates a row in a table. All applicable values in the given object
    * will be updated in the given table, according to its schema. If the
    * given "where" object is not NULL, its applicable members will define
    * the WHERE clause of the UPDATE SQL. An optional LIMIT amount may be
    * specified.
    * 
    * @param table the name of the table to UPDATE.
    * @param row the object with data to use in the update.
    * @param where the object with containing WHERE clause parameters.
    * @param limit 0 for no LIMIT, something positive to specify a LIMIT.
    * @param start the starting row for the LIMIT, defaults to 0.
    * @param affectedRows if not NULL, will store the number of affected rows.
    * @param c the connection to use, NULL to obtain one from the pool.
    * 
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool update(
      const char* table, db::rt::DynamicObject& row,
      db::rt::DynamicObject* where = NULL,
      uint64_t limit = 0, uint64_t start = 0,
      uint64_t* affectedRows = NULL, Connection* c = NULL);
   
   /**
    * Selects all column values not present in the given row object from
    * the specified table, using any present values in the WHERE clause
    * of the SELECT. 
    * 
    * @param table the name of the table to select from.
    * @param row the object to store the row result in, will be set to NULL
    *           if the SELECT returns back no rows.
    * @param c the connection to use, NULL to obtain one from the pool.
    * 
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool selectOne(
      const char* table, db::rt::DynamicObject& row, Connection* c = NULL);
   
   /**
    * Selects all column values not present in the given WHERE object from
    * the specified table, using any present values in the WHERE clause
    * of the SELECT. Each row result will be stored in the array rows. An
    * optional LIMIT amount may be specified.
    * 
    * @param table the name of the table to SELECT FROM.
    * @param rows the array object to store the rows result in.
    * @param where an object that specifies specific column values to
    *           look for, NULL to include no WHERE clause.
    * @param limit 0 for no LIMIT, something positive to specify a LIMIT.
    * @param start the starting row for the LIMIT, defaults to 0.
    * @param c the connection to use, NULL to obtain one from the pool.
    * 
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool select(
      const char* table, db::rt::DynamicObject& rows,
      db::rt::DynamicObject* where = NULL,
      uint64_t limit = 0, uint64_t start = 0,
      Connection* c = NULL);
   
   /**
    * Removes rows from a table. If the given "where" object is not NULL, its
    * applicable members will define the WHERE clause of the UPDATE SQL.
    * 
    * @param table the name of the table to DELETE FROM.
    * @param where the object with containing WHERE clause parameters.
    * @param affectedRows if not NULL, will store the number of affected rows.
    * @param c the connection to use, NULL to obtain one from the pool.
    * 
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool remove(
      const char* table, db::rt::DynamicObject* where,
      uint64_t* affectedRows = NULL, Connection* c = NULL);
   
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
   
protected:
   /**
    * Logs the passed SQL string if debug logging is on.
    * 
    * @param sql the SQL string to log.
    * @param params any params for the sql.
    */
   virtual void logSql(std::string& sql, db::rt::DynamicObject* params = NULL);
   
   /**
    * Converts a map of member-named values into an array of parameters. Each
    * member name that is recognized in the passed input object will be
    * converted into a parameter, that includes its associated table column
    * name, and it will be appended to the passed parameters array.
    * 
    * @param schema the table schema to use.
    * @param members the input map of member-named values.
    * @param params the parameters array to append to.
    */
   virtual void buildParams(
      SchemaObject& schema,
      db::rt::DynamicObject& members, db::rt::DynamicObject& params);
   
   /**
    * Builds an array of column schema information for columns based on the
    * given map of member-named values. The resulting column schemas array
    * can either be a collection of columns that do not appear the in the
    * given members object or a collection of columns that do appear. This
    * is defined by the "exclude" boolean parameter.
    * 
    * Each entry in the resulting array is for a single column and contains
    * a reference to that column's schema information that includes column
    * name, type, and associated member name and type. This can be used
    * to generate the columns for a SELECT statement and to pull data out
    * of the rows that were selected and put it into an object.
    * 
    * @param schema the table schema to use.
    * @param members the input map of member-named values.
    * @param columnSchemas the column schemas array to populate.
    * @param exclude true to add all column schemas except for those in
    *           "members", false to only include column schemas for those
    *           in "members".  
    */
   virtual void buildColumnSchemas(
      SchemaObject& schema,
      db::rt::DynamicObject* members, db::rt::DynamicObject& columnSchemas,
      bool exclude);
   
   /**
    * Appends the SQL " (col1,col2,...) VALUES (val1,val2,...)" to an SQL
    * statement.
    * 
    * @param sql the SQL string to append to.
    * @param params the list of parameters to generate the SQL from.
    */
   virtual void appendValuesSql(
      std::string& sql, db::rt::DynamicObject& params);
   
   /**
    * Appends the SQL " col1,col2,..." to an SQL statement.
    * 
    * @param sql the SQL string to append to.
    * @param columnSchemas the column schemas array to generate the SQL from.
    */
   virtual void appendColumnNames(
      std::string& sql, db::rt::DynamicObject& columnSchemas);
   
   /**
    * Appends the SQL " WHERE col1=? AND col2=? ..." to an SQL statement.
    * 
    * @param sql the SQL string to append to.
    * @param params the list of parameters to generate the SQL from.
    */
   virtual void appendWhereSql(
      std::string& sql, db::rt::DynamicObject& params);
   
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
    * Appends the SQL " SET col1=?,col2=? ..." to an SQL statement.
    * 
    * @param sql the SQL string to append to.
    * @param params the list of parameters to generate the SQL from.
    */
   virtual void appendSetSql(
      std::string& sql, db::rt::DynamicObject& params);
   
   /**
    * Sets the parameters for a statement.
    * 
    * @param s the statement.
    * @param params the parameters.
    * 
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool setParams(Statement* s, db::rt::DynamicObject& params);
   
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
      db::rt::DynamicObject& columnSchemas, Row* r, db::rt::DynamicObject& row);
   
   /**
    * Inserts or replaces a row into a table. All applicable values in the
    * given object will be inserted into the given table, according to its
    * schema.
    * 
    * @param cmd the command (INSERT or REPLACE).
    * @param table the name of the table to INSERT/REPLACE INTO.
    * @param row the object with data to insert as a row.
    * @param c the connection to use, NULL to obtain one from the pool.
    * 
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool insertOrReplace(
      const char* cmd, const char* table, db::rt::DynamicObject& row,
      Connection* c = NULL);
};

} // end namespace sql
} // end namespace db
#endif
