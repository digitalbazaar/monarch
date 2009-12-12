/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/AbstractConnection.h"

#include "monarch/sql/Statement.h"

#include <cstring>

using namespace std;
using namespace monarch::net;
using namespace monarch::sql;
using namespace monarch::rt;

AbstractConnection::AbstractConnection() :
   mUrl(NULL)
{
}

AbstractConnection::~AbstractConnection()
{
}

bool AbstractConnection::connect(const char* url)
{
   bool rval = false;

   // clean up old url
   mUrl.setNull();

   // ensure URL isn't malformed
   Exception::clear();
   mUrl = new Url(url);
   if(Exception::isSet())
   {
      ExceptionRef e = new Exception(
         "Invalid database url.",
         "monarch.sql.Connection.InvalidUrl");
      e->getDetails()["url"] = url;
      Exception::push(e);
   }
   else
   {
      // call implementation-specific code
      rval = connect(&(*mUrl));
   }

   return rval;
}

Statement* AbstractConnection::prepare(const char* sql)
{
   Statement* rval = getPreparedStatement(sql);
   if(rval == NULL)
   {
      // create statement
      rval = createStatement(sql);
      if(rval != NULL)
      {
         // add prepared statement
         addPreparedStatement(rval);
      }
   }

   return rval;
}

void AbstractConnection::close()
{
   // clean up url
   mUrl.setNull();

   // clean up prepared statements
   cleanupPreparedStatements();
}

bool AbstractConnection::begin()
{
   bool rval = false;

   Statement* s = prepare("BEGIN");
   rval = (s != NULL) && s->execute() && s->reset();
   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Could not begin transaction.",
         "monarch.sql.Connection.TransactionBeginError");
      Exception::push(e);
   }

   return rval;
}

bool AbstractConnection::commit()
{
   bool rval = false;

   Statement* s = prepare("COMMIT");
   rval = (s != NULL) && s->execute() && s->reset();
   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Could not commit transaction.",
         "monarch.sql.Connection.TransactionCommitError");
      Exception::push(e);
   }

   return rval;
}

bool AbstractConnection::rollback()
{
   bool rval = false;

   // save the reason for the rollback
   ExceptionRef reason = Exception::get();

   // attempt to do the rollback
   Statement* s = prepare("ROLLBACK");
   rval = (s != NULL) && s->execute() && s->reset();
   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Could not rollback transaction.",
         "monarch.sql.Connection.TransactionRollbackError");
      if(!reason.isNull())
      {
         e->getDetails()["rollbackReason"] =
            Exception::convertToDynamicObject(reason);
      }
      Exception::push(e);
   }

   return rval;
}

void AbstractConnection::cleanupPreparedStatements()
{
   // clean up all prepared statements
   for(PreparedStmtMap::iterator i = mPreparedStmts.begin();
       i != mPreparedStmts.end(); i++)
   {
      delete i->second;
   }
   mPreparedStmts.clear();
}

void AbstractConnection::addPreparedStatement(Statement* stmt)
{
   PreparedStmtMap::iterator i = mPreparedStmts.find(stmt->getSql());
   if(i != mPreparedStmts.end())
   {
      // delete old statement
      Statement* old = i->second;
      mPreparedStmts.erase(i);
      delete old;
   }

   // insert new statement
   mPreparedStmts.insert(make_pair(stmt->getSql(), stmt));
}

Statement* AbstractConnection::getPreparedStatement(const char* sql)
{
   Statement* rval = NULL;

   PreparedStmtMap::iterator i = mPreparedStmts.find(sql);
   if(i != mPreparedStmts.end())
   {
      rval = i->second;

      // reset statement for reuse
      if(!i->second->reset())
      {
         // reset failed, delete old statement
         mPreparedStmts.erase(i);
         delete rval;
         rval = NULL;
      }
   }

   return rval;
}
