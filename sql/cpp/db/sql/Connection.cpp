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
   mBeginStmt = NULL;
   mCommitStmt = NULL;
   mRollbackStmt = NULL;
}

Connection::~Connection()
{
   if(mUrl != NULL)
   {
      delete mUrl;
   }
   
   if(mBeginStmt != NULL)
   {
      delete mBeginStmt;
   }
   
   if(mCommitStmt != NULL)
   {
      delete mCommitStmt;
   }
   
   if(mRollbackStmt != NULL)
   {
      delete mRollbackStmt;
   }
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
#include <iostream>
bool Connection::begin()
{
   bool rval = false;
   std::cout << "++++++++++++++++++++++++++++NOOOOOOOOOOO!" << std::endl;
   
   if(mBeginStmt == NULL)
   {
      mBeginStmt = prepare("BEGIN");
   }
   
   if(mBeginStmt != NULL)
   {
      rval = mBeginStmt->execute();
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
   
   if(mCommitStmt == NULL)
   {
      mCommitStmt = prepare("COMMIT");
   }
   
   if(mCommitStmt != NULL)
   {
      rval = mCommitStmt->execute();
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
   
   if(mRollbackStmt == NULL)
   {
      mRollbackStmt = prepare("ROLLBACK");
   }
   
   if(mRollbackStmt != NULL)
   {
      rval = mRollbackStmt->execute();
   }
   else
   {
      ExceptionRef e = new SqlException("Could not rollback transaction!");
      Exception::setLast(e, true);
   }
   
   return rval;
}
