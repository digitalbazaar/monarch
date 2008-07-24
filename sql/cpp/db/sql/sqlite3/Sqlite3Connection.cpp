/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/sqlite3/Sqlite3Connection.h"

#include "db/sql/sqlite3/Sqlite3Statement.h"

using namespace std;
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
      string urlStr = url->toString();
      int length = 120 + urlStr.length();
      char msg[length];
      snprintf(msg, length,
         "Could not connect to sqlite3 database, "
         "url scheme doesn't start with 'sqlite3', url='%s'", urlStr.c_str());
      ExceptionRef e = new SqlException(msg, "db.sql.BadUrlScheme");
      Exception::setLast(e, false);
   }
   else
   {
      // get database name
      const char* db;
      if(strcmp(url->getScheme().c_str(), "sqlite3::memory:") == 0)
      {
         db = url->getScheme().c_str();
      }
      else
      {
         db = url->getPath().c_str();
      }
      
      // open sqlite3 connection
      int ec = sqlite3_open(db, &mHandle);
      if(ec != SQLITE_OK)
      {
         // create exception, close connection
         ExceptionRef e = new Sqlite3Exception(this);
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
