/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/sql/mysql/MySqlConnection.h"

#include "db/sql/mysql/MySqlStatement.h"

#include <mysql/errmsg.h>

using namespace std;
using namespace db::sql;
using namespace db::sql::mysql;
using namespace db::net;
using namespace db::rt;

MySqlConnection::MySqlConnection()
{
   // no handle yet
   mHandle = NULL;
}

MySqlConnection::~MySqlConnection()
{
   // ensure connection is closed
   MySqlConnection::close();
}

inline ::MYSQL* MySqlConnection::getHandle()
{
   return mHandle;
}

bool MySqlConnection::connect(Url* url)
{
   bool rval = false;

   if(strcmp(url->getScheme().c_str(), "mysql") != 0)
   {
      ExceptionRef e = new Exception(
         "Could not connect to mysql database. "
         "Url scheme doesn't start with 'mysql'",
         "db.sql.BadUrlScheme");
      e->getDetails()["url"] = url->toString().c_str();
      Exception::set(e);
   }
   else
   {
      // initialize handle
      mHandle = mysql_init(NULL);

      // default character set to UTF-8
      mysql_options(mHandle, MYSQL_SET_CHARSET_NAME, "utf8");

      // setup client flag
      int clientFlag = CLIENT_FOUND_ROWS | CLIENT_COMPRESS;

      // determine default database (if any)
      const char* db = (url->getPath().length() <= 1 ?
         NULL : url->getPath().c_str() + 1);

      // FIXME: we want to add read/write/create params to the URL
      // so connections can be read-only/write/etc (use query in URL)
      if(mysql_real_connect(
         mHandle,
         url->getHost().c_str(),
         url->getUser().c_str(), url->getPassword().c_str(),
         db,
         url->getPort(), NULL, clientFlag) == NULL)
      {
         // create exception, close connection
         ExceptionRef e = new MySqlException(this);
         Exception::set(e);
         MySqlConnection::close();
      }
      else
      {
         // connected
         rval = true;
      }
   }

   return rval;
}

void MySqlConnection::close()
{
   AbstractConnection::close();

   if(mHandle != NULL)
   {
      mysql_close(mHandle);
      mHandle = NULL;
   }
}

bool MySqlConnection::begin()
{
   bool rval;

   if(!(rval = (mysql_query(mHandle, "START TRANSACTION") == 0)))
   {
      ExceptionRef e = new Exception("Could not begin transaction.");
      ExceptionRef cause = new MySqlException(this);
      e->setCause(cause);
      Exception::set(e);
   }

   return rval;
}

bool MySqlConnection::commit()
{
   bool rval;

   if(!(rval = (mysql_query(mHandle, "COMMIT") == 0)))
   {
      ExceptionRef e = new Exception("Could not commit transaction.");
      ExceptionRef cause = new MySqlException(this);
      e->setCause(cause);
      Exception::set(e);
   }

   return rval;
}

bool MySqlConnection::rollback()
{
   bool rval;

   if(!(rval = (mysql_query(mHandle, "ROLLBACK") == 0)))
   {
      ExceptionRef e = new Exception("Could not rollback transaction.");
      ExceptionRef cause = new MySqlException(this);
      e->setCause(cause);
      Exception::set(e);
   }

   return rval;
}

bool MySqlConnection::isConnected()
{
   bool rval = false;

   if(mHandle != NULL)
   {
      rval = (mysql_ping(mHandle) == 0);
   }

   return rval;
}

bool MySqlConnection::setCharacterSet(const char* cset)
{
   // FIXME: handle exceptions
   mysql_options(mHandle, MYSQL_SET_CHARSET_NAME, cset);
   return true;
}

bool MySqlConnection::query(const char* sql)
{
   bool rval;

   if(!(rval = (mysql_query(mHandle, sql) == 0)))
   {
      ExceptionRef e = new Exception("Could not execute query.");
      ExceptionRef cause = new MySqlException(this);
      e->setCause(cause);
      Exception::set(e);
   }

   return rval;
}

Statement* MySqlConnection::createStatement(const char* sql)
{
   // create statement
   MySqlStatement* rval = new MySqlStatement(sql);
   if(!rval->initialize(this))
   {
      // delete statement if it could not be initialized
      delete rval;
      rval = NULL;
   }

   return rval;
}
