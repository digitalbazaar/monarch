/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_Row_h
#define db_database_Row_h

#include "db/database/Statement.h"
#include "db/database/DatabaseException.h"

#define DB_DATABASE_NULL 0
#define DB_DATABASE_INT 1
#define DB_DATABASE_TEXT 2

namespace db
{
namespace database
{

/**
 * A Row is a single result row from an executed database statement.
 * 
 * @author David I. Lehn
 * @author Dave Longley
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
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* getType(int column, int& type) = 0;
   
   /**
    * Gets an integer from a column.
    * 
    * @param column the column's index.
    * @param i the integer to store the integer in.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* getInteger(int column, int& i) = 0;
   
   /**
    * Gets a text string from a column.
    * 
    * @param column the column's index.
    * @param str the string to store the text in.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* getText(int column, std::string& str) = 0;
};

} // end namespace database
} // end namespace db
#endif
