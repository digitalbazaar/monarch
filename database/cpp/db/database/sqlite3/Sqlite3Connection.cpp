/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/database/sqlite3/Sqlite3Connection.h"
#include "db/database/sqlite3/Sqlite3Statement.h"

#include <iostream>

using namespace std;
using namespace db::database;
using namespace db::database::sqlite3;
using namespace db::rt;

Sqlite3Connection::Sqlite3Connection(const char* url) : Connection(url)
{
   // initialize handle
   mHandle = NULL;
   
   if(strcmp(mUrl.getScheme().c_str(), "sqlite3") == 0)
   {
      string msg;
      string urlStr;
      msg.append("Could not connect to sqlite3 database, ");
      msg.append("url scheme not 'sqlite3', url='");
      msg.append(mUrl.toString(urlStr));
      msg.append(1, '\'');
      
      Exception::setLast(new DatabaseException(msg.c_str()));
   }
   else
   {
      // FIXME: we want to add read/write/create params to the URL
      // so connections can be read-only/write/etc
      int ret = sqlite3_open(mUrl.getSchemeSpecificPart().c_str(), &mHandle);
      if(ret != SQLITE_OK)
      {
         Sqlite3Connection::close();
         
         
         
         // FIXME error handling
      }
   }
}

Sqlite3Connection::~Sqlite3Connection()
{
   Sqlite3Connection::close();
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
