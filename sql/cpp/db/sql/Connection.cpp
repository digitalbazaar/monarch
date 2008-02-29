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

bool Connection::begin()
{
   if(mBeginStmt == NULL)
   {
      mBeginStmt = prepare("BEGIN");
   }
   
   return mBeginStmt->execute();
}

bool Connection::commit()
{
   if(mCommitStmt == NULL)
   {
      mCommitStmt = prepare("COMMIT");
   }
   
   return mCommitStmt->execute();
}

bool Connection::rollback()
{
   if(mRollbackStmt == NULL)
   {
      mRollbackStmt = prepare("ROLLBACK");
   }
   
   return mRollbackStmt->execute();   
}
