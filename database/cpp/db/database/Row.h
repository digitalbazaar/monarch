/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_Row_h
#define db_database_Row_h

#include "db/database/Statement.h"

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
    *
    * @return the type ID for the column.
    */
   virtual int getType(int column) = 0;
   
   /**
    * Gets an integer from a column.
    * 
    * @param column the column's index.
    * 
    * @return the integer from the specified column.
    */
   virtual int getInteger(int column) = 0;
   
   /**
    * Gets a text string from a column.
    * 
    * @param column the column's index.
    * 
    * @return the text string from the specified column.
    */
   virtual const char* getText(int column) = 0;
};

} // end namespace database
} // end namespace db
#endif
