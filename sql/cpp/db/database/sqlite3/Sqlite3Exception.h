/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_sqlite3_Sqlite3Exception_H
#define db_database_sqlite3_Sqlite3Exception_H

#include "db/database/DatabaseException.h"

namespace db
{
namespace database
{
namespace sqlite3
{

// forward declare Sqlite3Connection
class Sqlite3Connection;

/**
 * A Sqlite3Exception is raised when some kind of sqlite3 database error
 * occurs. The sqlite3 connection must be passed to the constructor.
 * 
 * @author Dave Longley
 */
class Sqlite3Exception : public db::database::DatabaseException
{
public:
   /**
    * Creates a new Sqlite3Exception.
    * 
    * @param c the Sqlite3Connection the Exception occurred on.
    */
   Sqlite3Exception(Sqlite3Connection* c);
   
   /**
    * Destructs this Sqlite3Exception.
    */
   virtual ~Sqlite3Exception();
};

} // end namespace sqlite3
} // end namespace database
} // end namespace db
#endif
