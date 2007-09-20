/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_RowIterator_H
#define db_database_RowIterator_H

#include "db/util/Iterator.h"
#include "db/database/Row.h"

namespace db
{
namespace database
{

/**
 * A RowIterator is an Iterator for Statement Rows.
 * 
 * @author David I. Lehn
 */
class RowIterator : public db::util::Iterator<Row>
{
protected:
   /**
    * The statement.
    */
   Statement* mStatement;
   
public:
   /**
    * Creates a new RowIterator for the given stl list.
    * 
    * @param l the list to iterate over.
    */
   RowIterator(Statement* s);
   
   /**
    * Destructs this RowIterator.
    */
   virtual ~RowIterator();
   
   /**
    * Gets the next object and advances the RowIterator.
    * 
    * @return the next object.
    */
   virtual Row& next() = 0;
   
   /**
    * Returns true if this RowIterator has more objects.
    * 
    * @return true if this RowIterator has more objects, false if not.
    */
   virtual bool hasNext() = 0;
   
   /**
    * Removes the current object and advances the RowIterator.
    */
   virtual void remove() = 0;
};

} // end namespace database
} // end namespace db
#endif
