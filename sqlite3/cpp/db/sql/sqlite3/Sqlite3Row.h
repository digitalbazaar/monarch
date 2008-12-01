/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_sqlite3_Sqlite3Row_h
#define db_sql_sqlite3_Sqlite3Row_h

#include <sqlite3.h>

#include "db/sql/Row.h"

namespace db
{
namespace sql
{
namespace sqlite3
{

// forward declarations
class Sqlite3Connection;
class Sqlite3Statement;

/**
 * An Sqlite3Row is a Statement result Row for an sqlite3 database.
 * 
 * @author Dave Longley
 * @author David I. Lehn
 */
class Sqlite3Row : public db::sql::Row
{
protected:
   /**
    * The number of columns.
    */
   int mColumnCount;
   
   /**
    * Gets the C handle for the Sqlite3Statement.
    * 
    * @return the C handle for the Sqlite3Statement.
    */
   virtual sqlite3_stmt* getStatementHandle();
   
   /**
    * Gets the column index for the given column name.
    * 
    * @param name the name of the column.
    * 
    * @return the column index for the given column name of -1 if the name
    *         is invalid and an exception has been set.
    */
   int getColumnIndex(const char* name);
   
public:
   /**
    * Creates a new Sqlite3Row from the given Sqlite3Statement.
    * 
    * @param s the Sqlite3Statement to create the Sqlite3Row from.
    */
   Sqlite3Row(Sqlite3Statement* s);
   
   /**
    * Destructs this Row.
    */
   virtual ~Sqlite3Row();
   
   /**
    * Gets a column's data type.
    * 
    * @param column the column's index.
    * @param type the type ID for the column.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getType(unsigned int column, int& type);
   
   /**
    * Gets a 32-bit integer from a column.
    * 
    * @param column the column's index.
    * @param i the integer to store the integer in.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getInt32(unsigned int column, int& i);
   
   /**
    * Gets a 32-bit unsigned integer from a column.
    * 
    * @param column the column's index.
    * @param i the integer to store the integer in.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getUInt32(unsigned int column, unsigned int& i);
   
   /**
    * Gets a 64-bit integer from a column.
    * 
    * @param column the column's index.
    * @param i the integer to store the integer in.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getInt64(unsigned int column, long long& i);
   
   /**
    * Gets a 64-bit unsigned integer from a column.
    * 
    * @param column the column's index.
    * @param i the integer to store the integer in.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getUInt64(unsigned int column, unsigned long long& i);
   
   /**
    * Gets a text string from a column.
    * 
    * @param column the column's index.
    * @param str the string to store the text in.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getText(unsigned int column, std::string& str);
   
   /**
    * Gets a column's data type.
    *
    * @param column the column's name.
    * @param type the type ID for the column.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getType(const char* column, int& type);
   
   /**
    * Gets a 32-bit integer from a column.
    * 
    * @param column the column's name.
    * @param i the integer to store the integer in.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getInt32(const char* column, int& i);
   
   /**
    * Gets a 32-bit unsigned integer from a column.
    * 
    * @param column the column's name.
    * @param i the integer to store the integer in.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getUInt32(const char* column, unsigned int& i);
   
   /**
    * Gets a 64-bit integer from a column.
    * 
    * @param column the column's name.
    * @param i the integer to store the integer in.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getInt64(const char* column, long long& i);
   
   /**
    * Gets a 64-bit unsigned integer from a column.
    * 
    * @param column the column's name.
    * @param i the integer to store the integer in.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getUInt64(const char* column, unsigned long long& i);
   
   /**
    * Gets a text string from a column.
    * 
    * @param column the column's name.
    * @param str the string to store the text in.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getText(const char* column, std::string& str);
};

} // end namespace sqlite3
} // end namespace sql
} // end namespace db
#endif
