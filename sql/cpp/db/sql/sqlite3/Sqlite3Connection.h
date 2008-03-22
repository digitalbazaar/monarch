/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_sqlite3_Sqlite3Connection_H
#define db_sql_sqlite3_Sqlite3Connection_H

#include <sqlite3.h>

#include "db/sql/Connection.h"
#include "db/sql/sqlite3/Sqlite3Exception.h"

namespace db
{
namespace sql
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
class Sqlite3Connection : public db::sql::Connection
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
   
   /**
    * Creates a prepared Statement.
    * 
    * @param sql the standard query language text of the Statement.
    * 
    * @return the new Statement, NULL if an exception occurred.
    */
   virtual Statement* createStatement(const char* sql);
   
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
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool connect(db::net::Url* url);
   using db::sql::Connection::connect;
   
   /**
    * Closes this connection.
    */
   virtual void close();
   
   /**
    * Returns true if this connection is connected, false if not.
    * 
    * @return true if this connection is connected, false if not.
    */
   virtual bool isConnected();
};

} // end namespace sqlite3
} // end namespace sql
} // end namespace db
#endif
