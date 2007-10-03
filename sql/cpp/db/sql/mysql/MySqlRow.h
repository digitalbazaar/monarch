/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_mysql_MySqlRow_h
#define db_sql_mysql_MySqlRow_h

#include <mysql/mysql.h>

#include "db/sql/Row.h"

namespace db
{
namespace sql
{
namespace mysql
{

// forward declarations
class MySqlConnection;
class MySqlStatement;

/**
 * An MySqlRow is a Statement result Row for an mysql database.
 * 
 * @author Dave Longley
 */
class MySqlRow : public db::sql::Row
{
protected:
   /**
    * The fields for this row.
    */
   MYSQL_FIELD* mFields;
   
   /**
    * The number of fields.
    */
   unsigned int mFieldCount;
   
   /**
    * The bindings for this row.
    */
   MYSQL_BIND* mBindings;
   
   /**
    * Gets the C handle for the MySqlStatement.
    * 
    * @return the C handle for the MySqlStatement.
    */
   virtual MYSQL_STMT* getStatementHandle();
   
public:
   /**
    * Creates a new MySqlRow from the given MySqlStatement.
    * 
    * @param s the MySqlStatement to create the MySqlRow from.
    */
   MySqlRow(MySqlStatement* s);
   
   /**
    * Destructs this Row.
    */
   virtual ~MySqlRow();
   
   /**
    * Sets the fields for this row.
    * 
    * @param fields the fields for this row.
    * @param bindings the result bindings for this row.
    * @param count the number of fields.
    */
   virtual void setFields(
      MYSQL_FIELD* fields, unsigned int count, MYSQL_BIND* bindings);
   
   /**
    * Gets a column's data type.
    * 
    * @param column the column's index.
    * @param type the type ID for the column.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* getType(int column, int& type);
   
   /**
    * Gets a 32-bit integer from a column.
    * 
    * @param column the column's index.
    * @param i the integer to store the integer in.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* getInt32(int column, int& i);
   
   /**
    * Gets a 64-bit integer from a column.
    * 
    * @param column the column's index.
    * @param i the integer to store the integer in.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* getInt64(int column, long long& i);
   
   /**
    * Gets a text string from a column.
    * 
    * @param column the column's index.
    * @param str the string to store the text in.
    * 
    * @return an SqlException if one occurred, NULL if not.
    */
   virtual SqlException* getText(int column, std::string& str);
};

} // end namespace mysql
} // end namespace sql
} // end namespace db
#endif
