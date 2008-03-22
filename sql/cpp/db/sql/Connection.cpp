/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/Connection.h"
#include "db/sql/Statement.h"

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
   
   // clean up all prepared statements
   for(PreparedStmtMap::iterator i = mPreparedStmts.begin();
       i != mPreparedStmts.end(); i++)
   {
      delete i->second;
   }
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
   }
   
   return rval;
}

bool Connection::connect(const char* url)
{
   bool rval = false;
   
   // clean up old url
   if(mUrl != NULL)
   {
      delete mUrl;
   }
   
   // ensure URL isn't malformed
   Exception::clearLast();
   mUrl = new Url(url);
   if(Exception::hasLast())
   {
      string msg;
      msg.append("Invalid database url!,url=");
      msg.append(url);
      ExceptionRef e = new SqlException(msg.c_str());
      Exception::setLast(e, true);
   }
   else
   {
      // call implementation-specific code
      rval = connect(mUrl);
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
      ExceptionRef e = new SqlException("Could not begin transaction!");
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
      ExceptionRef e = new SqlException("Could not commit transaction!");
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
      ExceptionRef e = new SqlException("Could not rollback transaction!");
      Exception::setLast(e, true);
   }
   
   return rval;
}
