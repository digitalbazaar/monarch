/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_sqlite3_Statement_H
#define db_database_sqlite3_Statement_H

#include <sqlite3.h>

#include "db/database/Statement.h"
#include "db/database/sqlite3/Sqlite3RowIterator.h"

namespace db
{
namespace database
{
namespace sqlite3
{

class Sqlite3Connection;

/**
 * An sqlite3 database statement.
 * 
 * @author David I. Lehn
 * @author Dave Longley
 */
class Sqlite3Statement : public db::database::Statement
{
protected:
   /**
    * The C sqlite3 statement structure.
    */
   sqlite3_stmt* mHandle;
   
   /**
    * RowIterator for statement results;
    */
   Sqlite3RowIterator mRowIterator;
   
   /**
    * Sqlite3Row is a friend to allow access to the C handle.
    */
   friend class Sqlite3Row;
   
   /**
    * Sqlite3RowIterators is a friend to allow access to the C handle.
    */
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
    * @param pos the parameter position.
    * @param value the parameter value.
    */
   virtual void setInteger(int pos, int value);
   
   /**
    * Set a text string for a positional parameter.
    *
    * @param pos the parameter position.
    * @param value the parameter value.
    */
   virtual void setText(int pos, const char* value);
   
   /**
    * Executes this Statement.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* execute();
   
   /**
    * Gets the number of rows modified by this Statement.
    * 
    * @param rows to store the number of rows modified by this Statement.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* getRowsChanged(int& rows);
};

} // end namespace sqlite3
} // end namespace database
} // end namespace db
#endif
