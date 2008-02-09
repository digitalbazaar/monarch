/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/mysql/MySqlConnection.h"
#include "db/sql/mysql/MySqlStatement.h"

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

bool MySqlConnection::connect(Url* url)
{
   bool rval = false;
   
   if(strcmp(url->getScheme().c_str(), "mysql") != 0)
   {
      string urlStr;
      url->toString(urlStr);
      int length = 120 + urlStr.length();
      char msg[length];
      snprintf(msg, length,
         "Could not connect to sqlite3 database, "
         "url scheme doesn't start with 'sqlite3', url='%s'", urlStr.c_str());
      ExceptionRef e = new SqlException(msg);
      Exception::setLast(e, false);
   }
   else
   {
      // initialize handle
      mHandle = mysql_init(NULL);
      
      // default character set to UTF-8
      mysql_options(mHandle, MYSQL_SET_CHARSET_NAME, "utf8");
      
      // FIXME: we want to add read/write/create params to the URL
      // so connections can be read-only/write/etc (use query in URL)
      if(mysql_real_connect(
         mHandle,
         url->getHost().c_str(),
         url->getUser().c_str(), url->getPassword().c_str(),
         url->getPath().c_str() + 1,
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

Statement* MySqlConnection::prepare(const char* sql)
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

void MySqlConnection::close()
{
   if(mHandle != NULL)
   {
      mysql_close(mHandle);
      mHandle = NULL;
   }
}

bool MySqlConnection::setCharacterSet(const char* cset)
{
   // FIXME: handle exceptions
   mysql_options(mHandle, MYSQL_SET_CHARSET_NAME, cset);
   return true;
}
