/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_Connection_H
#define db_sql_Connection_H

#include "db/net/Url.h"
#include "db/sql/SqlException.h"
#include "db/rt/Collectable.h"

namespace db
{
namespace sql
{

// forward declare Statement
class Statement;

/**
 * A Connection is an abstract base class for a connection to a specific
 * type of database. Extending classes will provide appropriate implementation
 * details.
 * 
 * @author Dave Longley
 * @author David I. Lehn
 */
class Connection
{
protected:
   /**
    * The database driver parameters in URL form for this connection.
    */
   db::net::Url* mUrl;
   
   /**
    * An SqlComparator compares two sql statements.
    */
   typedef struct NameComparator
   {
      /**
       * Compares two null-terminated strings, returning true if the first is
       * less than the second, false if not. The compare is case-insensitive.
       * 
       * @param s1 the first string.
       * @param s2 the second string.
       * 
       * @return true if the s1 < s2, false if not.
       */
      bool operator()(const char* s1, const char* s2) const
      {
         return strcasecmp(s1, s2) < 0;
      }
   };
   
   /**
    * A map of sql to prepared statements for this connection.
    */
   typedef std::map<const char*, Statement*, NameComparator> PreparedStmtMap;
   PreparedStmtMap mPreparedStmts;
   
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
    * @return sql the sql of the statement to retrieve.
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
   
public:
   /**
    * Creates a new Connection.
    */
   Connection();
   
   /**
    * Destructs this Connection.
    */
   virtual ~Connection();
   
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
   virtual bool connect(db::net::Url* url) = 0;
   
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
   virtual void close() = 0;
   
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
    * Returns true if this connection is connected, false if not.
    * 
    * @return true if this connection is connected, false if not.
    */
   virtual bool isConnected() = 0;
};

// type definition for reference counted Connection
typedef db::rt::Collectable<Connection> ConnectionRef;

} // end namespace sql
} // end namespace db
#endif
