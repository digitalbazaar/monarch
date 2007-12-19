/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
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

SqlException* Connection::begin()
{
   if(mBeginStmt == NULL)
   {
      mBeginStmt = prepare("BEGIN");
   }
   
   return mBeginStmt->execute();
}

SqlException* Connection::commit()
{
   if(mCommitStmt == NULL)
   {
      mCommitStmt = prepare("COMMIT");
   }
   
   return mCommitStmt->execute();
}

SqlException* Connection::rollback()
{
   if(mRollbackStmt == NULL)
   {
      mRollbackStmt = prepare("ROLLBACK");
   }
   
   return mRollbackStmt->execute();   
}
