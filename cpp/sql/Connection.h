/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_Connection_H
#define monarch_sql_Connection_H

#include "monarch/rt/Exception.h"
#include "monarch/util/Url.h"

namespace monarch
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
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool connect(const char* url) = 0;

   /**
    * Connects to the database specified by the given url.
    *
    * @param url the url for the database to connect to, including driver
    *            specific parameters.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool connect(monarch::util::Url* url) = 0;

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
    * Prepares a printf formatted Statement for execution. The Statement, if
    * valid, is stored along with the Connection according to its sql. It's
    * memory is handled internally.
    *
    * @param format the printf-formatted string.
    *
    * @return the new stored Statement, NULL if an exception occurred.
    */
   virtual Statement* preparef(const char* format, ...)
#ifdef __GNUC__
      __attribute__ ((format (printf, 2, 3)))
#endif
         = 0;

   /**
    * Prepares a printf formatted Statement for execution. The Statement, if
    * valid, is stored along with the Connection according to its sql. It's
    * memory is handled internally.
    *
    * @param format the printf-formatted string.
    * @param varargs the variable length arguments.
    *
    * @return the new stored Statement, NULL if an exception occurred.
    */
   virtual Statement* vpreparef(const char* format, va_list varargs) = 0;

   /**
    * Closes this connection.
    */
   virtual void close() = 0;

   /**
    * Begins a new transaction.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool begin() = 0;

   /**
    * Commits the current transaction.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool commit() = 0;

   /**
    * Rolls back the current transaction.
    *
    * @return true if successful, false if an Exception occurred.
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
} // end namespace monarch
#endif
