/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_Statement_H
#define db_sql_Statement_H

#include "db/sql/Connection.h"

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
    * The connection associated with this statement.
    */
   Connection* mConnection;
   
   /**
    * The SQL for this statement.
    */
   char* mSql;
   
public:
   /**
    * Creates a new Statement.
    */
   Statement(Connection* c, const char* sql);
   
   /**
    * Destructs this Statement.
    */
   virtual ~Statement();
   
   /**
    * Gets the Connection that prepared this Statement.
    *
    * @return the Connection that prepared this Statement.
    */
   virtual Connection* getConnection();
   
   /**
    * Sets the value of a 32-bit integer for a positional parameter.
    * 
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setInt32(unsigned int param, int value) = 0;
   
   /**
    * Sets the value of a 32-bit unsigned integer for a positional parameter.
    * 
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setUInt32(unsigned int param, unsigned int value) = 0;
   
   /**
    * Sets the value of a 64-bit integer for a positional parameter.
    * 
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setInt64(unsigned int param, long long value) = 0;
   
   /**
    * Sets the value of a 64-bit unsigned integer for a positional parameter.
    * 
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setUInt64(
      unsigned int param, unsigned long long value) = 0;
   
   /**
    * Sets the value of a text string for a positional parameter.
    * 
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setText(
      unsigned int param, const char* value) = 0;
   
   /**
    * Sets the value of a 32-bit integer for a named parameter (:mynamehere).
    * 
    * @param name the parameter name.
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setInt32(const char* name, int value) = 0;
   
   /**
    * Sets the value of a 32-bit unsigned integer for a named parameter
    * (:mynamehere).
    * 
    * @param name the parameter name.
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setUInt32(const char* name, unsigned int value) = 0;
   
   /**
    * Sets the value of a 64-bit integer for a named parameter (:mynamehere).
    * 
    * @param name the parameter name.
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setInt64(const char* name, long long value) = 0;
   
   /**
    * Sets the value of a 64-bit unsigned integer for a named parameter
    * (:mynamehere).
    * 
    * @param name the parameter name.
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setUInt64(
      const char* name, unsigned long long value) = 0;
   
   /**
    * Sets the value of a text string for a named parameter (:mynamehere).
    * 
    * @param name the parameter name.
    * @param value parameter value.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* setText(const char* name, const char* value) = 0;
   
   /**
    * Executes this Statement.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* execute() = 0;
   
   /**
    * Fetches the next result Row once this Statement has been executed.
    * 
    * @return the next result Row once this Statement has been executed,
    *         NULL if there is no next Row.
    */
   virtual Row* fetch() = 0;
   
   /**
    * Gets the number of rows modified by this Statement.
    * 
    * @param rows to store the number of rows modified by this Statement.
    * 
    * @return a SqlException if one occurred, NULL if not.
    */
   virtual SqlException* getRowsChanged(unsigned long long& rows) = 0;
   
   /**
    * Gets the ID of the last row that was inserted. This is done per
    * connection and is useful for auto-incrementing rows.
    * 
    * @return the ID of the last row that was inserted.
    */
   virtual unsigned long long getLastInsertRowId() = 0;
};

} // end namespace sql
} // end namespace db
#endif
