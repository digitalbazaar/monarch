/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_database_DatabaseException_H
#define db_database_DatabaseException_H

#include "db/rt/Exception.h"

namespace db
{
namespace database
{

/**
 * A DatabaseException is raised when some kind of database error occurs.
 * 
 * @author Dave Longley
 */
class DatabaseException : public db::rt::Exception
{
protected:
   /**
    * The sql state associated with this exception, if any.
    */
   char* mSqlState;
   
public:
   /**
    * Creates a new DatabaseException.
    *
    * A message, type, and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param type the type for this Exception.
    * @param code the code for this Exception.
    */
   DatabaseException(
      const char* message = "",
      const char* type = "db.database.Database", int code = 0);
   
   /**
    * Destructs this DatabaseException.
    */
   virtual ~DatabaseException();
   
   /**
    * Sets the sql state string to associate with this exception.
    * 
    * @param state the sql state string to associate with this exception.
    */
   virtual void setSqlState(const char* state);
   
   /**
    * Gets the sql state string to associate with this exception.
    * 
    * @return the sql state string to associate with this exception.
    */
   virtual const char* getSqlState();
};

} // end namespace database
} // end namespace db
#endif
