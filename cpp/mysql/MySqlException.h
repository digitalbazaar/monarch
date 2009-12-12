/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_mysql_MySqlException_H
#define monarch_sql_mysql_MySqlException_H

#include "monarch/sql/SqlException.h"

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
 * A MySqlException is raised when some kind of mysql database error
 * occurs. The mysql connection or statement must be passed to the constructor.
 *
 * @author Dave Longley
 */
class MySqlException : public monarch::sql::SqlException
{
public:
   /**
    * Creates a new MySqlException.
    *
    * @param c the MySqlConnection the Exception occurred on.
    */
   MySqlException(MySqlConnection* c);

   /**
    * Creates a new MySqlException.
    *
    * @param s the MySqlStatement the Exception occurred during.
    */
   MySqlException(MySqlStatement* s);

   /**
    * Destructs this MySqlException.
    */
   virtual ~MySqlException();
};

} // end namespace mysql
} // end namespace sql
} // end namespace monarch
#endif
