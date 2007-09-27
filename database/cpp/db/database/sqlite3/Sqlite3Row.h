/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_sqlite3_Sqlite3Row_h
#define db_database_sqlite3_Sqlite3Row_h

#include <sqlite3.h>

#include "db/database/Row.h"
#include "db/database/sqlite3/Sqlite3Statement.h"

namespace db
{
namespace database
{
namespace sqlite3
{

// forward declarations
class Sqlite3Connection;
class Sqlite3Statement;

/**
 * An Sqlite3Row is a Statement result Row for an sqlite3 database.
 * 
 * @author David I. Lehn
 * @author Dave Longley
 */
class Sqlite3Row : public db::database::Row
{
protected:
   /**
    * Gets parent statements sqlite3_stmt.
    *
    * @return sqlite3_stmt for this Row
    */
   virtual sqlite3_stmt* getSqlite3Statement();
   
public:
   /**
    * Creates a new Row from the given Statement.
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
    *
    * @return the type ID for the column.
    */
   virtual int getType(int column);
   
   /**
    * Gets an integer from a column.
    * 
    * @param column the column's index.
    * 
    * @return the integer from the specified column.
    */
   virtual int getInteger(int column);
   
   /**
    * Gets a text string from a column.
    * 
    * @param column the column's index.
    * 
    * @return the text string from the specified column.
    */
   virtual const char* getText(int column);
};

} // end namespace sqlite3
} // end namespace database
} // end namespace db
#endif
