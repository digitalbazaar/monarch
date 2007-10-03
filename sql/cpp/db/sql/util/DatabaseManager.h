/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_util_DatabaseManager_H
#define db_sql_util_DatabaseManager_H

#include "db/sql/util/DatabaseClient.h"

namespace db
{
namespace sql
{
namespace util
{

/**
 * A DatabaseManager is used to provide a DatabaseClient for a particular
 * kind of database.
 * 
 * @author Dave Longley
 */
class DatabaseManager
{
protected:
   /**
    * Creates a new DatabaseManager.
    */
   DatabaseManager();
   
public:
   /**
    * Destructs this DatabaseManager.
    */
   virtual ~DatabaseManager();
   
   /**
    * Creates a DatabaseClient for the given url, where the url specifies
    * the database to connect to (and its protocol) as well as the appropriate
    * user permissions for connecting.
    * 
    * The returned DatabaseClient must be cleaned up by the caller of this
    * method.
    * 
    * @param url the url for the database to connect to, including its
    *            driver specific parameters.
    * 
    * @return the heap-allocated DatabaseClient or NULL if an exception
    *         occurred.
    */
   static DatabaseClient* createDatabaseClient(const char* url);
};

} // end namespace util
} // end namespace sql
} // end namespace db
#endif
