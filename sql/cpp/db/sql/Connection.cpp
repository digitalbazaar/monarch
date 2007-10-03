/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/Connection.h"

using namespace std;
using namespace db::net;
using namespace db::sql;
using namespace db::rt;

Connection::Connection()
{
   mUrl = NULL;
}

Connection::~Connection()
{
   if(mUrl != NULL)
   {
      delete mUrl;
   }
}

SqlException* Connection::connect(const char* url)
{
   SqlException* rval = NULL;
   
   // ensure URL isn't malformed
   Exception::clearLast();
   mUrl = new Url(url);
   if(Exception::hasLast())
   {
      string msg;
      msg.append("Invalid database url!,url=");
      msg.append(url);
      rval = new SqlException(msg.c_str());
      rval->setCause(Exception::getLast(), true);
      Exception::setLast(rval);
   }
   else
   {
      // call implementation-specific code
      rval = connect(mUrl);
   }
   
   return rval;
}

SqlException* Connection::commit()
{
   return (SqlException*)Exception::setLast(
      new SqlException("Connection::commit() not supported!"));
}

SqlException* Connection::rollback()
{
   return (SqlException*)Exception::setLast(
      new SqlException("Connection::rollback() not supported!"));
}
