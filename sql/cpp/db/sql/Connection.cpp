/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/Connection.h"

#include "db/sql/Statement.h"

using namespace std;
using namespace db::net;
using namespace db::sql;
using namespace db::rt;

Connection::Connection() :
   mUrl(NULL)
{
}

Connection::~Connection()
{
}

void Connection::addPreparedStatement(Statement* stmt)
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

Statement* Connection::getPreparedStatement(const char* sql)
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

bool Connection::connect(const char* url)
{
   bool rval = false;
   
   // clean up old url
   mUrl.setNull();
   
   // ensure URL isn't malformed
   Exception::clearLast();
   mUrl = new Url(url);
   if(Exception::hasLast())
   {
      ExceptionRef e = new Exception(
         "Invalid database url.",
         "db.sql.Connection.InvalidUrl");
      e->getDetails()["url"] = url;
      Exception::setLast(e, true);
   }
   else
   {
      // call implementation-specific code
      rval = connect(&(*mUrl));
   }
   
   return rval;
}

Statement* Connection::prepare(const char* sql)
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

void Connection::close()
{
   // clean up url
   mUrl.setNull();
   
   // clean up prepared statements
   cleanupPreparedStatements();
}

bool Connection::begin()
{
   bool rval = false;
   
   Statement* s = prepare("BEGIN");
   if(s != NULL)
   {
      rval = s->execute();
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not begin transaction.",
         "db.sql.Connection.TransactionBeginError");
      Exception::setLast(e, true);
   }
   
   return rval;
}

bool Connection::commit()
{
   bool rval = false;
   
   Statement* s = prepare("COMMIT");
   if(s != NULL)
   {
      rval = s->execute();
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not commit transaction.",
         "db.sql.Connection.TransactionCommitError");
      Exception::setLast(e, true);
   }
   
   return rval;
}

bool Connection::rollback()
{
   bool rval = false;
   
   Statement* s = prepare("ROLLBACK");
   if(s != NULL)
   {
      rval = s->execute();
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not rollback transaction.",
         "db.sql.Connection.TransactionRollbackError");
      Exception::setLast(e, true);
   }
   
   return rval;
}

void Connection::cleanupPreparedStatements()
{
   // clean up all prepared statements
   for(PreparedStmtMap::iterator i = mPreparedStmts.begin();
       i != mPreparedStmts.end(); i++)
   {
      delete i->second;
   }
   mPreparedStmts.clear();
}
