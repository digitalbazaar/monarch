/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_sql_Statement_H
#define db_sql_Statement_H

#include "monarch/sql/Connection.h"
#include "monarch/rt/Collectable.h"

namespace db
{
namespace sql
{

// forward declare row
class Row;

/**
 * A Statement is an abstract base class for SQL database statements. Extending
 * classes will provide the appropriate implementation details.
 *
 * @author Dave Longley
 * @author David I. Lehn
 */
class Statement
{
protected:
   /**
    * The SQL for this statement.
    */
   char* mSql;

public:
   /**
    * Creates a new Statement.
    *
    * @param sql the SQL for the statement.
    */
   Statement(const char* sql);

   /**
    * Destructs this Statement.
    */
   virtual ~Statement();

   /**
    * Gets the SQL for this statement.
    */
   virtual const char* getSql();

   /**
    * Gets the Connection that prepared this Statement.
    *
    * @return the Connection that prepared this Statement.
    */
   virtual Connection* getConnection() = 0;

   /**
    * Sets the value of a 32-bit integer for a positional parameter.
    *
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setInt32(unsigned int param, int32_t value) = 0;

   /**
    * Sets the value of a 32-bit unsigned integer for a positional parameter.
    *
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setUInt32(unsigned int param, uint32_t value) = 0;

   /**
    * Sets the value of a 64-bit integer for a positional parameter.
    *
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setInt64(unsigned int param, int64_t value) = 0;

   /**
    * Sets the value of a 64-bit unsigned integer for a positional parameter.
    *
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setUInt64(unsigned int param, uint64_t value) = 0;

   /**
    * Sets the value of a text string for a positional parameter.
    *
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setText(unsigned int param, const char* value) = 0;

   /**
    * Sets the value of a 32-bit integer for a named parameter (:mynamehere).
    *
    * @param name the parameter name.
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setInt32(const char* name, int32_t value) = 0;

   /**
    * Sets the value of a 32-bit unsigned integer for a named parameter
    * (:mynamehere).
    *
    * @param name the parameter name.
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setUInt32(const char* name, uint32_t value) = 0;

   /**
    * Sets the value of a 64-bit integer for a named parameter (:mynamehere).
    *
    * @param name the parameter name.
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setInt64(const char* name, int64_t value) = 0;

   /**
    * Sets the value of a 64-bit unsigned integer for a named parameter
    * (:mynamehere).
    *
    * @param name the parameter name.
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setUInt64(const char* name, uint64_t value) = 0;

   /**
    * Sets the value of a text string for a named parameter (:mynamehere).
    *
    * @param name the parameter name.
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setText(const char* name, const char* value) = 0;

   /**
    * Executes this Statement.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool execute() = 0;

   /**
    * Fetches the next result Row once this Statement has been executed. The
    * Row is managed by the Statement and must not be freed by the caller.
    *
    * @return the next result Row once this Statement has been executed,
    *         NULL if there is no next Row.
    */
   virtual Row* fetch() = 0;

   /**
    * Resets this statement for reuse.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool reset() = 0;

   /**
    * Gets the number of rows modified by this Statement.
    *
    * @param rows to store the number of rows modified by this Statement.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getRowsChanged(uint64_t& rows) = 0;

   /**
    * Gets the ID of the last row that was inserted. This is done per
    * connection and is useful for auto-incrementing rows.
    *
    * @return the ID of the last row that was inserted.
    */
   virtual uint64_t getLastInsertRowId() = 0;
};

// type definition for a reference counted Statement
typedef db::rt::Collectable<Statement> StatementRef;

} // end namespace sql
} // end namespace db
#endif
