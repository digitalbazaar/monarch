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

/**
 * Forward declaration.
 */
class Sqlite3Connection;
class Sqlite3Statement;

/**
 * A sqlite3 database statement.
 * 
 * @author David I. Lehn
 */
class Sqlite3Row : public db::database::Row
{
protected:
   /**
    * Get parent statements sqlite3_stmt.
    *
    * @return sqlite3_stmt for this Row
    */
   virtual sqlite3_stmt* getSqlite3Statement();

public:
   /**
    * Creates a new Row.
    */
   Sqlite3Row(Sqlite3Statement* s);
   
   /**
    * Destructs this Row.
    */
   virtual ~Sqlite3Row();

   /**
    * Get column type.
    *
    * @param col column index
    *
    * @return column type id
    */
   virtual int getType(int col);

   /**
    * Get int from column.
    *
    * @param col column index
    *
    * @return int from column
    */
   virtual int getInt(int col);

   /**
    * Get text string from column.
    *
    * @param col column index
    *
    * @return text string from column
    */
   virtual const char* getText(int col);
};

} // end namespace sqlite3
} // end namespace database
} // end namespace db
#endif
