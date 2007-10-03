/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_sql_SqlException_H
#define db_sql_SqlException_H

#include "db/rt/Exception.h"

namespace db
{
namespace sql
{

/**
 * A SqlException is raised when some kind of sql database error occurs.
 * 
 * @author Dave Longley
 */
class SqlException : public db::rt::Exception
{
protected:
   /**
    * The sql state associated with this exception, if any.
    */
   char* mSqlState;
   
public:
   /**
    * Creates a new SqlException.
    *
    * A message, type, and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param type the type for this Exception.
    * @param code the code for this Exception.
    */
   SqlException(
      const char* message = "",
      const char* type = "db.database.Database", int code = 0);
   
   /**
    * Destructs this SqlException.
    */
   virtual ~SqlException();
   
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

} // end namespace sql
} // end namespace db
#endif
