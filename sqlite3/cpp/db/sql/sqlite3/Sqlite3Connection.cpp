/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/sqlite3/Sqlite3Connection.h"

#include "db/sql/sqlite3/Sqlite3Statement.h"
#include "db/io/File.h"

using namespace std;
using namespace db::io;
using namespace db::sql;
using namespace db::sql::sqlite3;
using namespace db::net;
using namespace db::rt;

Sqlite3Connection::Sqlite3Connection()
{
   // initialize handle to NULL
   mHandle = NULL;
}

Sqlite3Connection::~Sqlite3Connection()
{
   // ensure connection is closed
   Sqlite3Connection::close();
}

Statement* Sqlite3Connection::createStatement(const char* sql)
{
   // create statement
   Exception::clearLast();
   Statement* rval = new Sqlite3Statement(this, sql);
   if(Exception::hasLast())
   {
      // delete statement if exception was thrown while creating statement
      delete rval;
      rval = NULL;
   }
   
   return rval;
}

bool Sqlite3Connection::connect(Url* url)
{
   bool rval = false;
   
   if(strncmp(url->getScheme().c_str(), "sqlite3", 7) != 0)
   {
      ExceptionRef e = new Exception(
         "Could not connect to sqlite3 database, url scheme doesn't "
         "start with 'sqlite3'.",
         "db.sql.BadUrlScheme");
      e->getDetails()["url"] = url->toString().c_str();
      Exception::setLast(e, false);
   }
   else
   {
      // get database name
      string db;
      if(strcmp(url->toString().c_str(), "sqlite3::memory:") == 0)
      {
         // use in-memory database
         db = ":memory:";
      }
      else
      {
         // use local file for database
         File file(url->getPath().c_str());
         db = file->getAbsolutePath();
      }
      
      // open sqlite3 connection
      int ec = sqlite3_open(db.c_str(), &mHandle);
      if(ec != SQLITE_OK)
      {
         // create exception, close connection
         ExceptionRef e = new Sqlite3Exception(this);
         e->getDetails()["url"] = url->toString().c_str();
         e->getDetails()["db"] = db.c_str();
         Exception::setLast(e, false);
         Sqlite3Connection::close();
      }
      else
      {
         // connected
         rval = true;
         
         // set busy timeout to 15 seconds
         sqlite3_busy_timeout(mHandle, 15000);
      }
   }
   
   return rval;
}

void Sqlite3Connection::close()
{
   Connection::close();
   
   if(mHandle != NULL)
   {
      sqlite3_close(mHandle);
      mHandle = NULL;
   }
}

inline bool Sqlite3Connection::isConnected()
{
   return mHandle != NULL;
}
