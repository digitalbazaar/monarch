/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_mysql_MySqlException_H
#define monarch_sql_mysql_MySqlException_H

#include "monarch/rt/Exception.h"

namespace monarch
{
namespace sql
{
namespace mysql
{

// forward declare MySqlConnection, MysqlStatement
class MySqlConnection;
class MySqlStatement;

/**
 * A MySqlException is raised when some kind of mysql database error occurs.
 * The mysql connection or statement must be passed to the create() function.
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
class MySqlException
{
protected:
   /**
    * Instances of MySqlExceptions cannot be created.
    */
   MySqlException();

public:
   /**
    * Destructs this MySqlException.
    */
   virtual ~MySqlException();

   /**
    * Creates a new MySql Exception.
    *
    * @param c the MySqlConnection the Exception occurred on.
    */
   static monarch::rt::Exception* create(MySqlConnection* c);

   /**
    * Creates a new MySql Exception.
    *
    * @param s the MySqlStatement the Exception occurred during.
    */
   static monarch::rt::Exception* create(MySqlStatement* s);
};

} // end namespace mysql
} // end namespace sql
} // end namespace monarch
#endif
