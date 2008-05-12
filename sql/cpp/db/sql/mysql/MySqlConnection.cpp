/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
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

Statement* MySqlConnection::createStatement(const char* sql)
{
   // create statement
   Exception::clearLast();
   Statement* rval = new MySqlStatement(this, sql);
   if(Exception::hasLast())
   {
      // delete statement if exception was thrown while creating statement
      delete rval;
      rval = NULL;
   }
   
   return rval;
}

bool MySqlConnection::connect(Url* url)
{
   bool rval = false;
   
   if(strcmp(url->getScheme().c_str(), "mysql") != 0)
   {
      string urlStr = url->toString();
      int length = 120 + urlStr.length();
      char msg[length];
      snprintf(msg, length,
         "Could not connect to mysql database, "
         "url scheme doesn't start with 'mysql', url='%s'", urlStr.c_str());
      ExceptionRef e = new SqlException(msg, "db.sql.BadUrlScheme");
      Exception::setLast(e, false);
   }
   else
   {
      // initialize handle
      mHandle = mysql_init(NULL);
      
      // default character set to UTF-8
      mysql_options(mHandle, MYSQL_SET_CHARSET_NAME, "utf8");
      
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
         url->getPort(), NULL, 0) == NULL)
      {
         // create exception, close connection
         ExceptionRef e = new MySqlException(this);
         Exception::setLast(e, false);
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
      ExceptionRef e = new SqlException("Could not begin transaction!");
      Exception::setLast(e, false);
   }
   
   return rval;
}

bool MySqlConnection::commit()
{
   bool rval;
   
   if(!(rval = (mysql_query(mHandle, "COMMIT") == 0)))
   {
      ExceptionRef e = new SqlException("Could not commit transaction!");
      Exception::setLast(e, false);
   }
   
   return rval;
}

bool MySqlConnection::rollback()
{
   bool rval;
   
   if(!(rval = (mysql_query(mHandle, "ROLLBACK") == 0)))
   {
      ExceptionRef e = new SqlException("Could not rollback transaction!");
      Exception::setLast(e, false);
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
