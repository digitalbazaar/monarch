/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_mysql_MySqlRow_h
#define monarch_sql_mysql_MySqlRow_h

#include <mysql/mysql.h>

#include "monarch/sql/Row.h"

namespace monarch
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
class MySqlRow : public monarch::sql::Row
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
    * {@inheritDoc}
    */
   virtual bool getType(unsigned int column, int& type);

   /**
    * {@inheritDoc}
    */
   virtual bool getInt32(unsigned int column, int32_t& i);

   /**
    * {@inheritDoc}
    */
   virtual bool getUInt32(unsigned int column, uint32_t& i);

   /**
    * {@inheritDoc}
    */
   virtual bool getInt64(unsigned int column, int64_t& i);

   /**
    * {@inheritDoc}
    */
   virtual bool getUInt64(unsigned int column, uint64_t& i);

   /**
    * {@inheritDoc}
    */
   virtual bool getText(unsigned int column, std::string& str);

   /**
    * {@inheritDoc}
    */
   virtual bool getBlob(unsigned int column, char* buffer, int* length);

   /**
    * {@inheritDoc}
    */
   virtual bool getType(const char* column, int& type);

   /**
    * {@inheritDoc}
    */
   virtual bool getInt32(const char* column, int32_t& i);

   /**
    * {@inheritDoc}
    */
   virtual bool getUInt32(const char* column, uint32_t& i);

   /**
    * {@inheritDoc}
    */
   virtual bool getInt64(const char* column, int64_t& i);

   /**
    * {@inheritDoc}
    */
   virtual bool getUInt64(const char* column, uint64_t& i);

   /**
    * {@inheritDoc}
    */
   virtual bool getText(const char* column, std::string& str);

   /**
    * {@inheritDoc}
    */
   virtual bool getBlob(const char* column, char* buffer, int* length);

protected:
   /**
    * Gets the column index for the given column name.
    *
    * @param name the name of the column.
    *
    * @return the column index for the given column name of -1 if the name
    *         is invalid and an exception has been set.
    */
   virtual int64_t getColumnIndex(const char* name);
};

} // end namespace mysql
} // end namespace sql
} // end namespace monarch
#endif
