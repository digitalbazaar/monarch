/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_sqlite3_Sqlite3Row_h
#define monarch_sql_sqlite3_Sqlite3Row_h

#include <sqlite3.h>

#include "monarch/sql/Row.h"

namespace monarch
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
class Sqlite3Row : public monarch::sql::Row
{
protected:
   /**
    * The number of columns.
    */
   int mColumnCount;

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
   virtual int getColumnIndex(const char* name);
};

} // end namespace sqlite3
} // end namespace sql
} // end namespace monarch
#endif
