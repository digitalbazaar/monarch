/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/mysql/MySqlException.h"

#include "monarch/sql/mysql/MySqlConnection.h"
#include "monarch/sql/mysql/MySqlStatement.h"

#include <mysql/mysql.h>

using namespace monarch::rt;
using namespace monarch::sql::mysql;

MySqlException::MySqlException()
{
}

MySqlException::~MySqlException()
{
}

Exception* MySqlException::create(MySqlConnection* c)
{
   Exception* e = new Exception(
      mysql_error(c->getHandle()),
      "monarch.sql.mysql.MySql",
      mysql_errno(c->getHandle()));
   e->getDetails()["sqlState"] = mysql_sqlstate(c->getHandle());
   return e;
}

Exception* MySqlException::create(MySqlStatement* s)
{
   Exception* e = new Exception(
      mysql_stmt_error(s->getHandle()),
      "monarch.sql.mysql.MySql",
      mysql_stmt_errno(s->getHandle()));
   e->getDetails()["sqlState"] = mysql_stmt_sqlstate(s->getHandle());
   return e;
}
