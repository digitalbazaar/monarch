/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_Statement_H
#define db_database_Statement_H

#include "db/database/Connection.h"

#define DB_DATABASE_UPDATE_ERROR -1 // executeUpdate error code

namespace db
{
namespace database
{

class RowIterator;

/**
 * A Statement is an abstract base class for SQL database statements. Extending
 * classes will provide the appropriate implementation details.
 * 
 * @author David I. Lehn
 * @author Dave Longley
 */
class Statement
{
protected:
   /**
    * The connection associated with this statement.
    */
   Connection* mConnection;
   
   /**
    * The SQL for this statement.
    */
   char* mSql;
   
public:
   /**
    * Creates a new Statement.
    */
   Statement(Connection* c, const char* sql);
   
   /**
    * Destructs this Statement.
    */
   virtual ~Statement();
   
   /**
    * Gets the Connection that prepared this Statement.
    *
    * @return the Connection that prepared this Statement.
    */
   virtual Connection* getConnection();
   
   /**
    * Set an integer for a positional parameter.
    *
    * @param pos parameter position.
    * @param value parameter value.
    */
   virtual void setInteger(int pos, int value) = 0;
   
   /**
    * Set a text string for a positional parameter.
    *
    * @param pos parameter position.
    * @param value parameter value.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual void setText(int pos, const char* value) = 0;
   
   /**
    * Executes this Statement.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* execute() = 0;
   
   /**
    * Gets the number of rows modified by this Statement.
    * 
    * @param rows to store the number of rows modified by this Statement.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* getRowsChanged(int& rows) = 0;
};

} // end namespace database
} // end namespace db
#endif
