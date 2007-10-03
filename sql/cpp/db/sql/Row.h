/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_Row_h
#define db_sql_Row_h

#include "db/sql/Statement.h"
#include "db/sql/SqlException.h"

namespace db
{
namespace sql
{

/**
 * A Row is a single result row from an executed database statement.
 * 
 * @author Dave Longley
 * @author David I. Lehn
 */
class Row
{
protected:
   /**
    * The associated Statement.
    */
   Statement* mStatement;
   
public:
   /**
    * Creates a new Row from a Statement.
    */
   Row(Statement* s);
   
   /**
    * Destructs this Row.
    */
   virtual ~Row();
   
   /**
    * Returns the statement associated with this row.
    */
   virtual Statement* getStatement();
   
   /**
    * Gets a column's data type.
    *
    * @param column the column's index.
    * @param type the type ID for the column.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* getType(int column, int& type) = 0;
   
   /**
    * Gets a 32-bit integer from a column.
    * 
    * @param column the column's index.
    * @param i the integer to store the integer in.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* getInt32(int column, int& i) = 0;
   
   /**
    * Gets a 64-bit integer from a column.
    * 
    * @param column the column's index.
    * @param i the integer to store the integer in.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* getInt64(int column, long long& i) = 0;
   
   /**
    * Gets a text string from a column.
    * 
    * @param column the column's index.
    * @param str the string to store the text in.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* getText(int column, std::string& str) = 0;
};

} // end namespace sql
} // end namespace db
#endif
