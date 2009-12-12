/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_sql_SqlException_H
#define monarch_sql_SqlException_H

#include "monarch/rt/Exception.h"

namespace monarch
{
namespace sql
{

/**
 * A SqlException is raised when some kind of sql database error occurs.
 *
 * @author Dave Longley
 */
class SqlException : public monarch::rt::Exception
{
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
      const char* type = "monarch.sql.Sql", int code = 0);

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
} // end namespace monarch
#endif
