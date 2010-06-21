/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_sqlite3_Sqlite3Exception_H
#define monarch_sql_sqlite3_Sqlite3Exception_H

#include "monarch/rt/Exception.h"

namespace monarch
{
namespace sql
{
namespace sqlite3
{

// forward declare Sqlite3Connection
class Sqlite3Connection;

/**
 * A Sqlite3Exception is raised when some kind of sqlite3 database error
 * occurs. The sqlite3 connection must be passed to the create() function.
 *
 * Since database access is typically added in as a module in monarch
 * applications, this class does not extend the Exception class. It is instead
 * used as a factory to create special types of exceptions. This prevents a
 * potential problem where the library that this class is part of is unloaded
 * after a database exception has been set. In that unfortunate case, the
 * necessary symbols for accessing or cleaning up the exception would be
 * unloaded and any attempted use of them would segfault the application.
 *
 * @author Dave Longley
 */
class Sqlite3Exception
{
protected:
   /**
    * Instances of Sqlite3Exceptions cannot be created.
    */
   Sqlite3Exception();

public:
   /**
    * Destructs this Sqlite3Exception.
    */
   virtual ~Sqlite3Exception();

   /**
    * Creates a new Sqlite3 Exception.
    *
    * @param c the Sqlite3Connection the Exception occurred on.
    */
   static monarch::rt::Exception* create(Sqlite3Connection* c);
};

} // end namespace sqlite3
} // end namespace sql
} // end namespace monarch
#endif
