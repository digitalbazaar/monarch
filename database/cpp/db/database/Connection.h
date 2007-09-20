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

// Forward declare Statement
class Statement;

/**
 * 
 * @author David I. Lehn
 */
class Connection
{
protected:
   db::net::Url* mInitParams;
   
   bool isOpen;
public:
   /**
    * Creates a new Connection.
    * 
    * @param params database driver specific parameters
    */
   Connection(const char* params);
   
   /**
    * Destructs this Connection.
    */
   virtual ~Connection();
   
   /**
    * Close connection.
    */
   virtual void close() = 0;

   /**
    * Commit current transaction.
    */
   virtual void commit();

   /**
    * Rollback current transaction.
    */
   virtual void rollback();
   
   /**
    * Close connection.
    *
    * @return new statement to be freed by caller.
    */
   virtual Statement* createStatement(const char* sql) = 0;
};

} // end namespace database
} // end namespace db
#endif
