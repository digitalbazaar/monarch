/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_mysql_MySqlRow_h
#define db_database_mysql_MySqlRow_h

#include <mysql/mysql.h>

#include "db/database/Row.h"

namespace db
{
namespace database
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
class MySqlRow : public db::database::Row
{
protected:
   /**
    * The current mysql row data.
    */
   MYSQL_ROW mData;
   
   /**
    * The number of columns.
    */
   unsigned int mColumnCount;
   
   /**
    * The lengths for each data field.
    */
   unsigned long* mLengths;
   
   /**
    * The fields for this row.
    */
   MYSQL_FIELD* mFields;
   
   /**
    * Gets the C handle for the MySqlStatement.
    * 
    * @return the C handle for the MySqlStatement.
    */
   virtual MYSQL_STMT* getStatementHandle();
   
   /**
    * Gets the column index for the given column name.
    * 
    * @param name the column name.
    * 
    * @return the column index for the given column name.
    */
   unsigned int getColumn(const char* name);
   
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
    * Sets the data for this row.
    * 
    * @param data the MYSQL_ROW for this row.
    * @param columns the number of columns for this row.
    * @param lengths the lengths for the data in each field.
    * @param field the field information.
    */
   virtual void setData(
      MYSQL_ROW& row, unsigned int columns, unsigned long* lengths,
      MYSQL_FIELD* fields);
   
   /**
    * Gets a column's data type.
    * 
    * @param column the column's index.
    * @param type the type ID for the column.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* getType(int column, int& type);
   
   /**
    * Gets a 32-bit integer from a column.
    * 
    * @param column the column's index.
    * @param i the integer to store the integer in.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* getInt32(int column, int& i);
   
   /**
    * Gets a 64-bit integer from a column.
    * 
    * @param column the column's index.
    * @param i the integer to store the integer in.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* getInt64(int column, long long& i);
   
   /**
    * Gets a text string from a column.
    * 
    * @param column the column's index.
    * @param str the string to store the text in.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* getText(int column, std::string& str);
};

} // end namespace mysql
} // end namespace database
} // end namespace db
#endif
