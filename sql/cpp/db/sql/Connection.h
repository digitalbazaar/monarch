/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_Connection_H
#define db_sql_Connection_H

#include "db/net/Url.h"
#include "db/sql/SqlException.h"

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
    * The begin statement.
    */
   Statement* mBeginStmt;
   
   /**
    * The commit statement.
    */
   Statement* mCommitStmt;
   
   /**
    * The rollback statement.
    */
   Statement* mRollbackStmt;
   
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
    * Prepares a Statement for execution. The Statement is heap-allocated and
    * must be freed by the caller of this method.
    * 
    * @param sql the standard query language text of the Statement.
    * 
    * @return the new Statement to be freed by caller, NULL if an
    *         exception occurred.
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
};

} // end namespace sql
} // end namespace db
#endif
