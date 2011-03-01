/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/mysql/MySqlConnection.h"

#include "monarch/data/json/JsonWriter.h"
#include "monarch/logging/Logging.h"
#include "monarch/sql/mysql/MySqlStatement.h"

#include <mysql/errmsg.h>
#include <cstdio>

using namespace std;
using namespace monarch::sql;
using namespace monarch::sql::mysql;
using namespace monarch::net;
using namespace monarch::rt;

MySqlConnection::MySqlConnection() :
   mHandle(NULL)
{
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
         "monarch.sql.BadUrlScheme");
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
         ExceptionRef e = createException();
         Exception::set(e);
         MO_CAT_ERROR(MO_SQL_CAT,
            "Could not connect to database host '%s:%d': %s",
            url->getHost().c_str(), url->getPort(),
            monarch::data::json::JsonWriter::writeToString(
               Exception::convertToDynamicObject(e)).c_str());
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

static Exception* _createException(::MYSQL* handle)
{
   Exception* e = new Exception(
      mysql_error(handle),
      "monarch.sql.mysql.MySql");
   e->getDetails()["code"] = mysql_errno(handle);
   e->getDetails()["sqlState"] = mysql_sqlstate(handle);
   return e;
}

static bool _query(::MYSQL* handle, const char* sql, const char* error)
{
   bool rval = (mysql_query(handle, sql) == 0);
   if(!rval)
   {
      ExceptionRef e = new Exception(
         error, "monarch.sql.mysql.MySql");
      ExceptionRef cause = _createException(handle);
      e->setCause(cause);
      Exception::set(e);
   }

   return rval;
}

bool MySqlConnection::begin()
{
   return _query(mHandle, "START TRANSACTION", "Could not begin transaction.");
}

bool MySqlConnection::commit()
{
   return _query(mHandle, "START TRANSACTION", "Could not commit transaction.");
}

bool MySqlConnection::rollback()
{
   return _query(mHandle, "ROLLBACK", "Could not rollback transaction.");
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
   return _query(mHandle, sql, "Could not execute query.");
}

bool MySqlConnection::setSqlMode(const char* mode)
{
   int len = strlen(mode) + 16;
   char sql[len];
   snprintf(sql, len, "SET sql_mode='%s'", mode);
   return _query(mHandle, sql, "Could not set mode.");
}

Exception* MySqlConnection::createException()
{
   return _createException(mHandle);
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
