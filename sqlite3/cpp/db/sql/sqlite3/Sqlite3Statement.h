/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_sql_sqlite3_Statement_H
#define db_sql_sqlite3_Statement_H

#include <sqlite3.h>

#include "db/sql/Statement.h"
#include "db/sql/sqlite3/Sqlite3Row.h"

namespace db
{
namespace sql
{
namespace sqlite3
{

class Sqlite3Connection;

/**
 * An sqlite3 database statement.
 *
 * @author Dave Longley
 * @author David I. Lehn
 */
class Sqlite3Statement : public db::sql::Statement
{
protected:
   /**
    * The C sqlite3 statement structure.
    */
   sqlite3_stmt* mHandle;

   /**
    * The current state for this statement, i.e. whether or not it
    * has been executed/whether or not a result Row is ready.
    */
   int mState;

   /**
    * The current row, if any.
    */
   Sqlite3Row* mRow;

public:
   /**
    * Creates a new Statement.
    */
   Sqlite3Statement(Sqlite3Connection* c, const char* sql);

   /**
    * Destructs this Statement.
    */
   virtual ~Sqlite3Statement();

   /**
    * Gets the sqlite3 handle for this statement.
    *
    * @return the sqlite3 handle for this statement.
    */
   virtual sqlite3_stmt* getHandle();

   /**
    * Initializes this statement for use.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool initialize();

   /**
    * Sets the value of a 32-bit integer for a positional parameter.
    *
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setInt32(unsigned int param, int32_t value);

   /**
    * Sets the value of a 32-bit unsigned integer for a positional parameter.
    *
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setUInt32(unsigned int param, uint32_t value);

   /**
    * Sets the value of a 64-bit integer for a positional parameter.
    *
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setInt64(unsigned int param, int64_t value);

   /**
    * Sets the value of a 64-bit unsigned integer for a positional parameter.
    *
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setUInt64(unsigned int param, uint64_t value);

   /**
    * Sets the value of a text string for a positional parameter.
    *
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setText(unsigned int param, const char* value);

   /**
    * Sets the value of a 32-bit integer for a named parameter (:mynamehere).
    *
    * @param name the parameter name.
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setInt32(const char* name, int32_t value);

   /**
    * Sets the value of a 32-bit unsigned integer for a named parameter
    * (:mynamehere).
    *
    * @param name the parameter name.
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setUInt32(const char* name, uint32_t value);

   /**
    * Sets the value of a 64-bit integer for a named parameter (:mynamehere).
    *
    * @param name the parameter name.
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setInt64(const char* name, int64_t value);

   /**
    * Sets the value of a 64-bit unsigned integer for a named parameter
    * (:mynamehere).
    *
    * @param name the parameter name.
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setUInt64(const char* name, uint64_t value);

   /**
    * Sets the value of a text string for a named parameter (:mynamehere).
    *
    * @param name the parameter name.
    * @param value parameter value.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setText(const char* name, const char* value);

   /**
    * Executes this Statement.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool execute();

   /**
    * Fetches the next result Row once this Statement has been executed. The
    * Row is heap-allocated and must be freed by the caller of this method.
    *
    * @return the next result Row once this Statement has been executed,
    *         NULL if there is no next Row.
    */
   virtual Row* fetch();

   /**
    * Resets this statement for reuse.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool reset();

   /**
    * Gets the number of rows modified by this Statement.
    *
    * @param rows to store the number of rows modified by this Statement.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getRowsChanged(uint64_t& rows);

   /**
    * Gets the ID of the last row that was inserted. This is done per
    * connection and is useful for auto-incrementing rows.
    *
    * @return the ID of the last row that was inserted.
    */
   virtual uint64_t getLastInsertRowId();

protected:
   /**
    * Gets the parameter index for the given name.
    *
    * @param name the name to get the parameter index for.
    *
    * @return the index >= 1 or 0 if an SqlException occurred.
    */
   virtual int getParameterIndex(const char* name);
};

} // end namespace sqlite3
} // end namespace sql
} // end namespace db
#endif
