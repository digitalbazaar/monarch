/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_AbstractConnection_H
#define monarch_sql_AbstractConnection_H

#include "monarch/sql/Connection.h"
#include "monarch/util/StringTools.h"

namespace monarch
{
namespace sql
{

/**
 * An AbstractConnection implements the basics of an sql Connection without
 * using any database specifics. This class provides a partial implementation
 * for commonly used database paradigms like prepared statements.
 *
 * @author Dave Longley
 */
class AbstractConnection : public Connection
{
protected:
   /**
    * The database driver parameters in URL form for this connection.
    */
   monarch::net::UrlRef mUrl;

   /**
    * A map of sql to prepared statements for this connection using
    * case-insensitive comparator to compare sql statements.
    */
   typedef std::map<const char*, Statement*, monarch::util::StringCaseComparator>
      PreparedStmtMap;
   PreparedStmtMap mPreparedStmts;

public:
   /**
    * Creates a new AbstractConnection.
    */
   AbstractConnection();

   /**
    * Destructs this AbstractConnection.
    */
   virtual ~AbstractConnection();

   /**
    * Connects to the database specified by the given url.
    *
    * @param url the url for the database to connect to, including driver
    *            specific parameters.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool connect(const char* url);

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
   virtual Statement* prepare(const char* sql);

   /**
    * Closes this connection.
    */
   virtual void close();

   /**
    * Begins a new transaction.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool begin();

   /**
    * Commits the current transaction.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool commit();

   /**
    * Rolls back the current transaction.
    *
    * @return true if successful, false if an SqlException occurred.
    */
   virtual bool rollback();

   /**
    * Cleans up this connection's prepared statements.
    */
   virtual void cleanupPreparedStatements();

protected:
   /**
    * Adds a heap-allocated statement to the map of prepared statements,
    * overwriting any existing one. The memory for the statement will be
    * auto-managed by this Connection.
    *
    * @param stmt the pre-compiled prepared statement.
    */
   virtual void addPreparedStatement(Statement* stmt);

   /**
    * Retrieves a previously stored prepared statement.
    *
    * @param sql the sql of the statement to retrieve.
    *
    * @return the pre-compiled prepared statement or NULL if not found.
    */
   virtual Statement* getPreparedStatement(const char* sql);

   /**
    * Creates a prepared Statement.
    *
    * @param sql the standard query language text of the Statement.
    *
    * @return the new Statement, NULL if an exception occurred.
    */
   virtual Statement* createStatement(const char* sql) = 0;
};

} // end namespace sql
} // end namespace monarch
#endif
