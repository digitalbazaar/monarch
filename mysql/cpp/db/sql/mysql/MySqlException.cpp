/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/mysql/MySqlException.h"
#include "db/sql/mysql/MySqlConnection.h"
#include "db/sql/mysql/MySqlStatement.h"

#include <mysql/mysql.h>

using namespace db::sql;
using namespace db::sql::mysql;

MySqlException::MySqlException(MySqlConnection* c) :
   SqlException("", "db.sql.mysql.MySql")
{
   setCode(mysql_errno(c->mHandle));
   setMessage(mysql_error(c->mHandle));
   setSqlState(mysql_sqlstate(c->mHandle));
}

MySqlException::MySqlException(MySqlStatement* s)
{
   setCode(mysql_stmt_errno(s->mHandle));
   setMessage(mysql_stmt_error(s->mHandle));
   setSqlState(mysql_stmt_sqlstate(s->mHandle));
}

MySqlException::~MySqlException()
{
}
