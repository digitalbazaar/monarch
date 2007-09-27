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
 * A RowIterator is an Iterator for Statement result Rows.
 * 
 * @author David I. Lehn
 * @author Dave Longley
 */
class RowIterator : public db::util::Iterator<Row>
{
protected:
   /**
    * The associated statement.
    */
   Statement* mStatement;
   
public:
   /**
    * Creates a new RowIterator for the given Statement.
    * 
    * @param s the Statement with result rows to iterate over.
    */
   RowIterator(Statement* s);
   
   /**
    * Destructs this RowIterator.
    */
   virtual ~RowIterator();
   
   /**
    * Gets the next Row and advances the RowIterator.
    * 
    * @return the next Row.
    */
   virtual Row& next() = 0;
   
   /**
    * Returns true if this RowIterator has more Rows.
    * 
    * @return true if this RowIterator has more Rows, false if not.
    */
   virtual bool hasNext() = 0;
   
   /**
    * Has no effect, as Rows cannot be removed.
    */
   virtual void remove() {};
};

} // end namespace database
} // end namespace db
#endif
