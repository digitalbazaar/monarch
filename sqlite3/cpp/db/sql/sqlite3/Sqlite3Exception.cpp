/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/sqlite3/Sqlite3Exception.h"
#include "db/sql/sqlite3/Sqlite3Connection.h"

#include <sqlite3.h>

using namespace db::sql;
using namespace db::sql::sqlite3;

Sqlite3Exception::Sqlite3Exception(Sqlite3Connection* c) :
   SqlException("", "db.sql.sqlite3.Sqlite3")
{
   setCode(sqlite3_errcode(c->getHandle()));
   setMessage(sqlite3_errmsg(c->getHandle()));
}

Sqlite3Exception::~Sqlite3Exception()
{
}
