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
 * A database statement result row.
 * 
 * @author David I. Lehn
 */
class Row
{
protected:
   Statement* mStatement;

public:
   /**
    * Creates a new Row.
    */
   Row(Statement* s);
   
   /**
    * Destructs this Row.
    */
   virtual ~Row();
   
   /**
    * Return the statement for this result set.
    */
   virtual Statement* getStatement();
   
   /**
    * Get column type.
    *
    * @param col column index
    *
    * @return column type id
    */
   virtual int getType(int col) = 0;

   /**
    * Get int from column.
    *
    * @param col column index
    *
    * @return int from column
    */
   virtual int getInt(int col) = 0;

   /**
    * Get text string from column.
    *
    * @param col column index
    *
    * @return text string from column
    */
   virtual const char* getText(int col) = 0;
};

} // end namespace database
} // end namespace db
#endif
