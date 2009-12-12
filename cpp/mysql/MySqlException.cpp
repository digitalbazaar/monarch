/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/mysql/MySqlException.h"

#include "monarch/sql/mysql/MySqlConnection.h"
#include "monarch/sql/mysql/MySqlStatement.h"

#include <mysql/mysql.h>

using namespace monarch::sql;
using namespace monarch::sql::mysql;

MySqlException::MySqlException(MySqlConnection* c) :
   SqlException("", "db.sql.mysql.MySql")
{
   setCode(mysql_errno(c->getHandle()));
   setMessage(mysql_error(c->getHandle()));
   setSqlState(mysql_sqlstate(c->getHandle()));
}

MySqlException::MySqlException(MySqlStatement* s)
{
   setCode(mysql_stmt_errno(s->getHandle()));
   setMessage(mysql_stmt_error(s->getHandle()));
   setSqlState(mysql_stmt_sqlstate(s->getHandle()));
}

MySqlException::~MySqlException()
{
}
