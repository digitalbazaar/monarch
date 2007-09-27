/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_sqlite3_Sqlite3Connection_H
#define db_database_sqlite3_Sqlite3Connection_H

#include <sqlite3.h>

#include "db/database/Connection.h"


namespace db
{
namespace database
{
namespace sqlite3
{

// forward declaration
class Sqlite3Statement;

/**
 * 
 * @author David I. Lehn
 */
class Sqlite3Connection : public db::database::Connection
{
protected:
   ::sqlite3* mHandle;
   friend class Sqlite3Statement;
public:
   /**
    * Creates a new Connection.
    *
    * @param params Sqlite3 parameters in URI form:
    *        "sqlite:/path/to/example.db" A SQLite3 database called example.db
    */
   Sqlite3Connection(const char* params);
   
   /**
    * Destructs this Connection.
    */
   virtual ~Sqlite3Connection();
   
   /**
    * Prepares a Statement for execution. The Statement is heap-allocated and
    * must be freed by the caller of this method.
    * 
    * @param sql the standard query language text of the Statement.
    * 
    * @return the new Statement to be freed by caller.
    */
   virtual Statement* prepareStatement(const char* sql);
   
   /**
    * Closes this connection.
    */
   virtual void close();
   
   /**
    * Commits the current transaction.
    */
   virtual void commit();
   
   /**
    * Rolls back the current transaction.
    */
   virtual void rollback();
};

} // end namespace sqlite3
} // end namespace database
} // end namespace db
#endif
