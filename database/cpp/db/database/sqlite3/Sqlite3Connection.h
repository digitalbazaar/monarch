/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_sqlite3_Sqlite3Connection_H
#define db_database_sqlite3_Sqlite3Connection_H

#include <sqlite3.h>

#include "db/database/Connection.h"
#include "db/database/sqlite3/Sqlite3Exception.h"

namespace db
{
namespace database
{
namespace sqlite3
{

// forward declaration
class Sqlite3Statement;

/**
 * An Sqlite3Connection is a Connection to an sqlite3 database.
 * 
 * @author Dave Longley
 * @author David I. Lehn
 */
class Sqlite3Connection : public db::database::Connection
{
protected:
   /**
    * The handle to the sqlite3 database.
    */
   ::sqlite3* mHandle;
   
   /**
    * These classes are friends so they can access the C handle to
    * the database.
    */
   friend class Sqlite3Statement;
   friend class Sqlite3Exception;
   
public:
   /**
    * Creates a new Connection.
    */
   Sqlite3Connection();
   
   /**
    * Destructs this Connection.
    */
   virtual ~Sqlite3Connection();
   
   /**
    * Connects to the database specified by the given url.
    * 
    * @param url Sqlite3 parameters in URL form:
    *        "sqlite://user:password@/path/to/example.db"
    *        A SQLite3 database called example.db
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* connect(const char* url);
   
   /**
    * Prepares a Statement for execution. The Statement is heap-allocated and
    * must be freed by the caller of this method.
    * 
    * @param sql the standard query language text of the Statement.
    * 
    * @return the new Statement to be freed by caller, NULL if an
    *         exception occurred.
    */
   virtual Statement* prepare(const char* sql);
   
   /**
    * Closes this connection.
    */
   virtual void close();
   
   /**
    * Commits the current transaction.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* commit();
   
   /**
    * Rolls back the current transaction.
    * 
    * @return a DatabaseException if one occurred, NULL if not.
    */
   virtual DatabaseException* rollback();
};

} // end namespace sqlite3
} // end namespace database
} // end namespace db
#endif
