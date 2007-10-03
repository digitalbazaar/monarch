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
   SqlException* rval = NULL;
   
   mUrl = new Url(url);
   if(strcmp(mUrl->getScheme().c_str(), "mysql") != 0)
   {
      string msg;
      string urlStr;
      msg.append("Could not connect to mysql database, ");
      msg.append("url scheme not 'mysql', url='");
      msg.append(mUrl->toString(urlStr));
      msg.append(1, '\'');
      
      Exception::setLast(new SqlException(msg.c_str()));
   }
   else
   {
      // FIXME: we want to add read/write/create params to the URL
      // so connections can be read-only/write/etc (use query in URL)
      if(mysql_real_connect(
         mHandle,
         mUrl->getHost().c_str(),
         mUrl->getUser().c_str(), mUrl->getPassword().c_str(),
         mUrl->getPath().c_str() + 1,
         mUrl->getPort(), NULL, 0) == NULL)
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
   return new MySqlStatement(this, sql);
}

void MySqlConnection::close()
{
   if(mHandle != NULL)
   {
      mysql_close(mHandle);
      mHandle = NULL;
   }
}

SqlException* MySqlConnection::commit()
{
   SqlException* rval = NULL;
   
   if(!mysql_commit(mHandle))
   {
      rval = new MySqlException(this);
      Exception::setLast(rval);
   }
   
   return rval;
}

SqlException* MySqlConnection::rollback()
{
   SqlException* rval = NULL;
   
   if(!mysql_rollback(mHandle))
   {
      rval = new MySqlException(this);
      Exception::setLast(rval);
   }
   
   return rval;
}

SqlException* MySqlConnection::setCharacterSet(const char* cset)
{
   // FIXME: handle exceptions
   mysql_options(mHandle, MYSQL_SET_CHARSET_NAME, cset);
   return NULL;
}
