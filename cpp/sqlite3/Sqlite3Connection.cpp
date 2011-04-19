/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/sqlite3/Sqlite3Connection.h"

#include "monarch/sql/sqlite3/Sqlite3Statement.h"
#include "monarch/io/File.h"

using namespace std;
using namespace monarch::io;
using namespace monarch::sql;
using namespace monarch::sql::sqlite3;
using namespace monarch::rt;
using namespace monarch::util;

Sqlite3Connection::Sqlite3Connection() :
   mHandle(NULL)
{
}

Sqlite3Connection::~Sqlite3Connection()
{
   // ensure connection is closed
   Sqlite3Connection::close();
}

inline ::sqlite3* Sqlite3Connection::getHandle()
{
   return mHandle;
}

bool Sqlite3Connection::connect(Url* url)
{
   bool rval = false;

   string scheme = url->getScheme();
   if(strncmp(scheme.c_str(), "sqlite3", 7) != 0 &&
      strncmp(scheme.c_str(), "file", 4) != 0)
   {
      ExceptionRef e = new Exception(
         "Could not connect to sqlite3 database, url scheme doesn't "
         "start with 'sqlite3' or 'file'.",
         "monarch.sql.BadUrlScheme");
      e->getDetails()["url"] = url->toString().c_str();
      Exception::set(e);
   }
   else
   {
      // get database name
      string db;
      if(strcmp(url->toString().c_str(), "sqlite3::memory:") == 0)
      {
         // use in-memory database
         db = ":memory:";
         rval = true;
      }
      else
      {
         // use local file for database
         File file(url->getPath().c_str());
         db = file->getAbsolutePath();
         rval = file->mkdirs();
      }

      if(rval)
      {
         // open sqlite3 connection, SQLITE_OPEN_NOMUTEX will allow
         // multi-threaded access to the database as opposed to
         // SQLITE_OPEN_FULLMUTEX which would serialize all access to
         // the database
         // FIXME: SQLITE_OPEN_NOMUTEX option disabled until OS distributions
         // like ubuntu catch up with sqlite 3.6
         /*
         int ec = sqlite3_open_v2(
            db.c_str(), &mHandle,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, | SQLITE_OPEN_NOMUTEX,
            NULL);
         */
         int ec = sqlite3_open(db.c_str(), &mHandle);
         if(ec != SQLITE_OK)
         {
            // create exception, close connection
            ExceptionRef e = createException();
            e->getDetails()["url"] = url->toString().c_str();
            e->getDetails()["db"] = db.c_str();
            Exception::set(e);
            Sqlite3Connection::close();
            rval = false;
         }
         else
         {
            // connected, set busy timeout to 15 seconds
            sqlite3_busy_timeout(mHandle, 15000);
         }
      }
   }

   return rval;
}

void Sqlite3Connection::close()
{
   AbstractConnection::close();

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

Exception* Sqlite3Connection::createException()
{
   Exception* e = new Exception(
      sqlite3_errmsg(mHandle),
      "monarch.sql.sqlite3.Sqlite3");
   e->getDetails()["code"] = sqlite3_errcode(mHandle);
   return e;
}

Statement* Sqlite3Connection::createStatement(const char* sql)
{
   // create statement
   Sqlite3Statement* rval = new Sqlite3Statement(sql);
   if(!rval->initialize(this))
   {
      // delete statement if it could not be initialized
      delete rval;
      rval = NULL;
   }

   return rval;
}
