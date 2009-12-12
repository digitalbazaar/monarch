/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_sql_Connection_H
#define db_sql_Connection_H

#include "monarch/net/Url.h"
#include "monarch/sql/SqlException.h"
#include "monarch/rt/Collectable.h"

namespace db
{
namespace sql
{

// forward declare Statement
class Statement;

/**
 * A Connection is an interface for a connection to a specific type of
 * database. Extending classes will provide appropriate implementation details.
 *
 * @author Dave Longley
 * @author David I. Lehn
 */
class Connection
{
public:
   /**
    * Creates a new Connection.
    */
   Connection() {};

   /**
    * Destructs this Connection.
    */
   virtual ~Connection() {};

   /**
    * Connects to the database specified by the given url.
    *
    * @param url the url for the database to connect to, including driver
    *            specific parameters.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool connect(const char* url) = 0;

   /**
    * Connects to the database specified by the given url.
    *
    * @param url the url for the database to connect to, including driver
    *            specific parameters.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool connect(monarch::net::Url* url) = 0;

   /**
    * Prepares a Statement for execution. The Statement, if valid, is stored
    * along with the Connection according to its sql. It's memory is handled
    * internally.
    *
    * @param sql the standard query language text of the Statement.
    *
    * @return the new stored Statement, NULL if an exception occurred.
    */
   virtual Statement* prepare(const char* sql) = 0;

   /**
    * Closes this connection.
    */
   virtual void close() = 0;

   /**
    * Begins a new transaction.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool begin() = 0;

   /**
    * Commits the current transaction.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool commit() = 0;

   /**
    * Rolls back the current transaction.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool rollback() = 0;

   /**
    * Returns true if this connection is connected, false if not.
    *
    * @return true if this connection is connected, false if not.
    */
   virtual bool isConnected() = 0;

   /**
    * Cleans up this connection's prepared statements.
    */
   virtual void cleanupPreparedStatements() = 0;
};

// type definition for reference counted Connection
typedef monarch::rt::Collectable<Connection> ConnectionRef;

} // end namespace sql
} // end namespace db
#endif
