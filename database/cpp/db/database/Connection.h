/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_Connection_H
#define db_database_Connection_H

#include "db/net/Url.h"

namespace db
{
namespace database
{

// forward declare Statement
class Statement;

/**
 * 
 * A Connection is an abstract base class for a connection to a specific
 * type of database. Extending classes will provide appropriate implementation
 * details.
 * 
 * @author David I. Lehn
 * @author Dave Longley
 */
class Connection
{
protected:
   /**
    * The database driver parameters for this connection.
    */
   db::net::Url* mDatabaseParams;
   
   /**
    * True if this connection is open, false if not.
    */
   bool mOpen;
   
public:
   /**
    * Creates a new Connection.
    * 
    * @param params database driver specific parameters.
    */
   Connection(const char* params);
   
   /**
    * Destructs this Connection.
    */
   virtual ~Connection();
   
   /**
    * Prepares a Statement for execution. The Statement is heap-allocated and
    * must be freed by the caller of this method.
    * 
    * @param sql the standard query language text of the Statement.
    * 
    * @return the new Statement to be freed by caller.
    */
   virtual Statement* prepareStatement(const char* sql) = 0;
   
   /**
    * Closes this connection.
    */
   virtual void close() = 0;
   
   /**
    * Commits the current transaction.
    */
   virtual void commit() {};
   
   /**
    * Rolls back the current transaction.
    */
   virtual void rollback() {};
};

} // end namespace database
} // end namespace db
#endif
