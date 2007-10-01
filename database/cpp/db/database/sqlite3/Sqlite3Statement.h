/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_sqlite3_Statement_H
#define db_database_sqlite3_Statement_H

#include <sqlite3.h>

#include "db/database/Statement.h"
#include "db/database/sqlite3/Sqlite3Row.h"

namespace db
{
namespace database
{
namespace sqlite3
{

class Sqlite3Connection;

/**
 * An sqlite3 database statement.
 * 
 * @author Dave Longley
 * @author David I. Lehn
 */
class Sqlite3Statement : public db::database::Statement
{
protected:
   /**
    * The C sqlite3 statement structure.
    */
   sqlite3_stmt* mHandle;
   
   /**
    * The current state for this statement, i.e. whether or not it
    * has been executed/whether or not a result Row is ready.
    */
   int mState;
   
   /**
    * The current row, if any.
    */
   Sqlite3Row* mRow;
   
   /**
    * Sqlite3Row is a friend to allow access to the C handle.
    */
   friend class Sqlite3Row;
   
public:
   /**
    * Creates a new Statement.
    */
   Sqlite3Statement(Sqlite3Connection* c, const char* sql);
   
   /**
    * Destructs this Statement.
    */
   virtual ~Sqlite3Statement();
   
   /**
    * Sets the value of a 32-bit integer for a positional parameter.
    * 
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* setInt32(int param, int value);
   
   /**
    * Sets the value of a 64-bit integer for a positional parameter.
    * 
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* setInt64(int param, long long value);
   
   /**
    * Sets the value of a text string for a positional parameter.
    * 
    * @param param the parameter number (1 being the first param).
    * @param value parameter value.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* setText(int param, const char* value);
   
   /**
    * Sets the value of a 32-bit integer for a named parameter (:mynamehere).
    * 
    * @param name the parameter name.
    * @param value parameter value.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* setInt32(const char* name, int value);
   
   /**
    * Sets the value of a 64-bit integer for a named parameter (:mynamehere).
    * 
    * @param name the parameter name.
    * @param value parameter value.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* setInt64(const char* name, long long value);
   
   /**
    * Sets the value of a text string for a named parameter (:mynamehere).
    * 
    * @param name the parameter name.
    * @param value parameter value.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* setText(const char* name, const char* value);
   
   /**
    * Executes this Statement.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* execute();
   
   /**
    * Fetches the next result Row once this Statement has been executed.
    * 
    * @return the next result Row once this Statement has been executed,
    *         NULL if there is no next Row.
    */
   virtual Row* fetch();
   
   /**
    * Gets the number of rows modified by this Statement.
    * 
    * @param rows to store the number of rows modified by this Statement.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* getRowsChanged(int& rows);
   
   /**
    * Gets the ID of the last row that was inserted. This is done per
    * connection and is useful for auto-incrementing rows.
    * 
    * @return the ID of the last row that was inserted.
    */
   virtual long long getLastInsertRowId();
};

} // end namespace sqlite3
} // end namespace database
} // end namespace db
#endif
