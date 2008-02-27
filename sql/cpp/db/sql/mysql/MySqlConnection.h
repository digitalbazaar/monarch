/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_mysql_MySqlConnection_H
#define db_sql_mysql_MySqlConnection_H

#include <mysql/mysql.h>

#include "db/sql/Connection.h"
#include "db/sql/mysql/MySqlException.h"

namespace db
{
namespace sql
{
namespace mysql
{

// forward declaration
class MySqlStatement;

/**
 * An MySqlConnection is a Connection to an mysql database.
 * 
 * @author Dave Longley
 */
class MySqlConnection : public db::sql::Connection
{
protected:
   /**
    * The handle to the mysql database.
    */
   ::MYSQL* mHandle;
   
   /**
    * These classes are friends so they can access the C handle to
    * the database.
    */
   friend class MySqlStatement;
   friend class MySqlException;
   
public:
   /**
    * Creates a new Connection.
    */
   MySqlConnection();
   
   /**
    * Destructs this Connection.
    */
   virtual ~MySqlConnection();
   
   /**
    * Connects to the specified host using the specified default database. No
    * default database will be set if no path is provided in the url.
    * 
    * @param url MySql parameters in URL form:
    *            "mysql://user:password@host:port/databasename"
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool connect(db::net::Url* url);
   using db::sql::Connection::connect;
   
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
    * Returns true if this connection is connected, false if not.
    * 
    * @return true if this connection is connected, false if not.
    */
   virtual bool isConnected();
   
   /**
    * Sets the character set for this connection.
    * 
    * @param cset the character set to use for this connection.
    * 
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool setCharacterSet(const char* cset);
};

} // end namespace mysql
} // end namespace sql
} // end namespace db
#endif
