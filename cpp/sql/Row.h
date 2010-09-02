/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_Row_h
#define monarch_sql_Row_h

#include "monarch/sql/Statement.h"

namespace monarch
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
    *
    * @param s the Statement for the Row.
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
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getType(unsigned int column, int& type) = 0;

   /**
    * Gets a 32-bit integer from a column.
    *
    * @param column the column's index.
    * @param i the integer to store the integer in.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getInt32(unsigned int column, int32_t& i) = 0;

   /**
    * Gets a 32-bit unsigned integer from a column.
    *
    * @param column the column's index.
    * @param i the integer to store the integer in.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getUInt32(unsigned int column, uint32_t& i) = 0;

   /**
    * Gets a 64-bit integer from a column.
    *
    * @param column the column's index.
    * @param i the integer to store the integer in.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getInt64(unsigned int column, int64_t& i) = 0;

   /**
    * Gets a 64-bit unsigned integer from a column.
    *
    * @param column the column's index.
    * @param i the integer to store the integer in.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getUInt64(unsigned int column, uint64_t& i) = 0;

   /**
    * Gets a text string from a column.
    *
    * @param column the column's index.
    * @param str the string to store the text in.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getText(unsigned int column, std::string& str) = 0;

   /**
    * Gets a blob from a column. If the given byte array is not large enough,
    * the length will still be set but the byte array will not be filled.
    *
    * @param column the column's index.
    * @param buffer the byte array to store the blob in.
    * @param length the length of the byte array, to be set to the length of
    *           the blob.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getBlob(unsigned int column, char* buffer, int* length) = 0;

   /**
    * Gets a column's data type.
    *
    * @param column the column's name.
    * @param type the type ID for the column.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getType(const char* column, int& type) = 0;

   /**
    * Gets a 32-bit integer from a column.
    *
    * @param column the column's name.
    * @param i the integer to store the integer in.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getInt32(const char* column, int32_t& i) = 0;

   /**
    * Gets a 32-bit unsigned integer from a column.
    *
    * @param column the column's name.
    * @param i the integer to store the integer in.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getUInt32(const char* column, uint32_t& i) = 0;

   /**
    * Gets a 64-bit integer from a column.
    *
    * @param column the column's name.
    * @param i the integer to store the integer in.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getInt64(const char* column, int64_t& i) = 0;

   /**
    * Gets a 64-bit unsigned integer from a column.
    *
    * @param column the column's name.
    * @param i the integer to store the integer in.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getUInt64(const char* column, uint64_t& i) = 0;

   /**
    * Gets a text string from a column.
    *
    * @param column the column's name.
    * @param str the string to store the text in.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getText(const char* column, std::string& str) = 0;

   /**
    * Gets a blob from a column. If the given byte array is not large enough,
    * the length will still be set but the byte array will not be filled.
    *
    * @param column the column's name.
    * @param buffer the byte array to store the blob in.
    * @param length the length of the byte array, to be set to the length of
    *           the blob.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool getBlob(const char* column, char* buffer, int* length) = 0;
};

} // end namespace sql
} // end namespace monarch
#endif
