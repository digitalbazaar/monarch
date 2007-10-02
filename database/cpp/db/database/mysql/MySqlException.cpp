/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/database/mysql/MySqlException.h"
#include "db/database/mysql/MySqlConnection.h"
#include "db/database/mysql/MySqlStatement.h"

#include <mysql/mysql.h>

using namespace db::database;
using namespace db::database::mysql;

MySqlException::MySqlException(MySqlConnection* c) :
   DatabaseException("", "db.database.mysql.MySql")
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
