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
 * A database statement.
 * 
 * @author David I. Lehn
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
   const char* mSql;

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
    * Retreive the statements connection.
    *
    * @return the Connection
    */
   virtual Connection* getConnection();
   
   /**
    * Set an integer for a positional parameter.
    *
    * @param pos parameter position
    * @param value parameter value
    */
   virtual void setInt(int pos, int value) = 0;

   /**
    * Set a text string for a positional parameter.
    *
    * @param pos parameter position
    * @param value parameter value
    */
   virtual void setText(int pos, const char* value) = 0;
   
   /**
    * Execute a SELECT statement.
    *
    * @return result set for the query, NULL on error.
    */
   virtual RowIterator* executeQuery() = 0;

   /**
    * Execute a UPDATE, INSERT, or DELETE statement.
    *
    * @return number of updated rows, DB_DATABASE_UPDATE_ERROR on error.
    */
   virtual int executeUpdate() = 0;

   virtual int getErrorCode() = 0;
   virtual const char* getErrorMessage() = 0;
};

} // end namespace database
} // end namespace db
#endif
