/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_sqlite3_Statement_H
#define db_database_sqlite3_Statement_H

#include <sqlite3.h>

#include "db/database/Statement.h"
#include "db/database/RowIterator.h"

namespace db
{
namespace database
{
namespace sqlite3
{

class Sqlite3Connection;

/**
 * A sqlite3 database statement.
 * 
 * @author David I. Lehn
 */
class Sqlite3Statement : public db::database::Statement
{
protected:
   /**
    * sqlite3 statement
    */
   sqlite3_stmt* mSqlite3Statement;

   /**
    * RowIterator for statement results;
    */
   RowIterator* mRowIterator;

   friend class Sqlite3Row;
   friend class Sqlite3RowIterator;

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
    * Set an integer for a positional parameter.
    *
    * @param pos parameter position
    * @param value parameter value
    */
   virtual void setInt(int pos, int value);

   /**
    * Set a text string for a positional parameter.
    *
    * @param pos parameter position
    * @param value parameter value
    */
   virtual void setText(int pos, const char* value);
   
   /**
    * Execute a SELECT statement.
    *
    * @return result set for the query, NULL on error.
    */
   virtual RowIterator* executeQuery();

   /**
    * Execute a UPDATE, INSERT, or DELETE statement.
    *
    * @return number of updated rows, -1 on error.
    */
   virtual int executeUpdate();

   virtual int getErrorCode();
   virtual const char* getErrorMessage();
};

} // end namespace sqlite3
} // end namespace database
} // end namespace db
#endif
