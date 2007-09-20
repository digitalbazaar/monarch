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
   
   virtual void close();
   virtual void commit();
   virtual void rollback();
   virtual Statement* createStatement(const char* sql);
};

} // end namespace sqlite3
} // end namespace database
} // end namespace db
#endif
