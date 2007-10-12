/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_mysql_Statement_H
#define db_sql_mysql_Statement_H

#include <mysql/mysql.h>

#include "db/sql/Statement.h"
#include "db/sql/mysql/MySqlRow.h"

namespace db
{
namespace sql
{
namespace mysql
{

class MySqlConnection;

/**
 * An mysql database statement.
 * 
 * @author Dave Longley
 */
class MySqlStatement : public db::sql::Statement
{
protected:
   /**
    * The C mysql statement structure.
    */
   MYSQL_STMT* mHandle;
   
   /**
    * The result meta-data for this statement, if any.
    */
   MYSQL_RES* mResult;
   
   /**
    * The number of parameters in this statement.
    */
   unsigned int mParamCount;
   
   /**
    * The parameter bindings for this statement.
    */
   MYSQL_BIND* mParamBindings;
   
   /**
    * The number of result fields for this statement.
    */
   unsigned int mFieldCount;
   
   /**
    * The result bindings for this statement.
    */
   MYSQL_BIND* mResultBindings;
   
   /**
    * The current row, if any.
    */
   MySqlRow* mRow;
   
   /**
    * MySqlRow is a friend to allow access to the C handle.
    */
   friend class MySqlRow;
   
   /**
    * MySqlException is a friend to allow access to the C handle.
    */
   friend class MySqlException;
   
public:
   /**
    * Creates a new Statement.
    */
   MySqlStatement(MySqlConnection* c, const char* sql);
   
   /**
    * Destructs this Statement.
    */
   virtual ~MySqlStatement();
   
   /**
    * Sets the value of a 32-bit integer for a positional parameter.
    * 
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setInt32(unsigned int param, int value);
   
   /**
    * Sets the value of a 32-bit unsigned integer for a positional parameter.
    * 
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setUInt32(unsigned int param, unsigned int value);
   
   /**
    * Sets the value of a 64-bit integer for a positional parameter.
    * 
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setInt64(unsigned int param, long long value);
   
   /**
    * Sets the value of a 64-bit unsigned integer for a positional parameter.
    * 
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setUInt64(
      unsigned int param, unsigned long long value);
   
   /**
    * Sets the value of a text string for a positional parameter.
    * 
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setText(unsigned int param, const char* value);
   
   /**
    * Sets the value of a 32-bit integer for a named parameter (:mynamehere).
    * 
    * @param name the parameter name.
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setInt32(const char* name, int value);
   
   /**
    * Sets the value of a 32-bit unsigned integer for a named parameter
    * (:mynamehere).
    * 
    * @param name the parameter name.
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setUInt32(const char* name, unsigned int value);
   
   /**
    * Sets the value of a 64-bit integer for a named parameter (:mynamehere).
    * 
    * @param name the parameter name.
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setInt64(const char* name, long long value);
   
   /**
    * Sets the value of a 64-bit unsigned integer for a named parameter
    * (:mynamehere).
    * 
    * @param name the parameter name.
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setUInt64(const char* name, unsigned long long value);
   
   /**
    * Sets the value of a text string for a named parameter (:mynamehere).
    * 
    * @param name the parameter name.
    * @param value parameter value.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setText(const char* name, const char* value);
   
   /**
    * Executes this Statement.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* execute();
   
   /**
    * Fetches the next result Row once this Statement has been executed.
    * 
    * @return the next result Row once this Statement has been executed,
    *         NULL if there is no next Row.
    */
   virtual Row* fetch();
   
   /**
    * Gets the number of rows modified by this Statement.
    * 
    * @param rows to store the number of rows modified by this Statement.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* getRowsChanged(unsigned long long& rows);
   
   /**
    * Gets the ID of the last row that was inserted. This is done per
    * connection and is useful for auto-incrementing rows.
    * 
    * @return the ID of the last row that was inserted.
    */
   virtual unsigned long long getLastInsertRowId();
};

} // end namespace mysql
} // end namespace sql
} // end namespace db
#endif
