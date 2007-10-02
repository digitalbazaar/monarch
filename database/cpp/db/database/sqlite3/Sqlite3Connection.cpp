/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/database/sqlite3/Sqlite3Connection.h"
#include "db/database/sqlite3/Sqlite3Statement.h"

#include <iostream>

using namespace std;
using namespace db::database;
using namespace db::database::sqlite3;
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

DatabaseException* Sqlite3Connection::connect(const char* url)
{
   DatabaseException* rval = NULL;
   
   mUrl = new Url(url);
   if(strncmp(mUrl->getScheme().c_str(), "sqlite3", 7) != 0)
   {
      string msg;
      string urlStr;
      msg.append("Could not connect to sqlite3 database, ");
      msg.append("url scheme doesn't start with 'sqlite3', url='");
      msg.append(mUrl->toString(urlStr));
      msg.append(1, '\'');
      
      Exception::setLast(new DatabaseException(msg.c_str()));
   }
   else
   {
      // FIXME: we want to add read/write/create params to the URL
      // so connections can be read-only/write/etc (use query in URL)
      // handle username/password
      int ec = sqlite3_open(mUrl->getSchemeSpecificPart().c_str(), &mHandle);
      if(ec != SQLITE_OK)
      {
         // create exception, close connection
         rval = new Sqlite3Exception(this);
         Exception::setLast(rval);
         Sqlite3Connection::close();
      }
   }
   
   return rval;
}

Statement* Sqlite3Connection::prepare(const char* sql)
{
   return new Sqlite3Statement(this, sql);
}

void Sqlite3Connection::close()
{
   if(mHandle != NULL)
   {
      sqlite3_close(mHandle);
      mHandle = NULL;
   }
}

DatabaseException* Sqlite3Connection::commit()
{
   // FIXME:
   cout << "FIXME: commit" << endl;
   return NULL;
}

DatabaseException* Sqlite3Connection::rollback()
{
   // FIXME:
   cout << "FIXME: rollback" << endl;
   return NULL;
}
