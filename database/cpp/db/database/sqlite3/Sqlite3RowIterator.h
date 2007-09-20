/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_sqlite3_Sqlite3RowIterator_h
#define db_database_sqlite3_Sqlite3RowIterator_h

#include "db/database/Row.h"
#include "db/database/RowIterator.h"
#include "db/database/sqlite3/Sqlite3Statement.h"

namespace db
{
namespace database
{
namespace sqlite3
{

class Sqlite3Statement;

/**
 * A RowIterator is an Iterator for Statement Rows.
 * 
 * @author David I. Lehn
 */
class Sqlite3RowIterator : public db::database::RowIterator
{
protected:
   /**
    * Current row.
    */
   Row* mRow;

public:
   /**
    * Creates a new RowIterator for the given stl list.
    * 
    * @param l the list to iterate over.
    */
   Sqlite3RowIterator(Sqlite3Statement* s);
   
   /**
    * Destructs this RowIterator.
    */
   virtual ~Sqlite3RowIterator();
   
   /**
    * Gets the next object and advances the RowIterator.
    * 
    * @return the next object.
    */
   virtual Row& next();
   
   /**
    * Returns true if this RowIterator has more objects.
    * 
    * @return true if this RowIterator has more objects, false if not.
    */
   virtual bool hasNext();
   
   /**
    * Removes the current object and advances the RowIterator.
    */
   virtual void remove();
};

} // end namespace sqlite3
} // end namespace database
} // end namespace db
#endif
