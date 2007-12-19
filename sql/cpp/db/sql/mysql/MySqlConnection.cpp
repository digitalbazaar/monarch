/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/mysql/MySqlConnection.h"
#include "db/sql/mysql/MySqlStatement.h"

#include <iostream>

using namespace std;
using namespace db::sql;
using namespace db::sql::mysql;
using namespace db::net;
using namespace db::rt;

MySqlConnection::MySqlConnection()
{
   // initialize handle
   mHandle = mysql_init(NULL);
   
   // default character set to UTF-8
   mysql_options(mHandle, MYSQL_SET_CHARSET_NAME, "utf8");
}

MySqlConnection::~MySqlConnection()
{
   // ensure connection is closed
   MySqlConnection::close();
}

SqlException* MySqlConnection::connect(const char* url)
{
   return Connection::connect(url);
}

SqlException* MySqlConnection::connect(Url* url)
{
   SqlException* rval = NULL;
   
   if(strcmp(url->getScheme().c_str(), "mysql") != 0)
   {
      string urlStr;
      url->toString(urlStr);
      
      char temp[100 + urlStr.length()];
      sprintf(
         temp, "Could not connect to mysql database, url scheme not "
         "'mysql', url='%s'", urlStr.c_str());
      Exception::setLast(new SqlException(temp));
   }
   else
   {
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
         rval = new MySqlException(this);
         Exception::setLast(rval);
         MySqlConnection::close();
      }
   }
   
   return rval;
}

Statement* MySqlConnection::prepare(const char* sql)
{
   Exception* e = Exception::getLast();
   
   // create statement
   Statement* rval = new MySqlStatement(this, sql);
   if(Exception::getLast() != e)
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

SqlException* MySqlConnection::setCharacterSet(const char* cset)
{
   // FIXME: handle exceptions
   mysql_options(mHandle, MYSQL_SET_CHARSET_NAME, cset);
   return NULL;
}
