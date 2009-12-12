/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/sqlite3/Sqlite3Exception.h"

#include "monarch/sql/sqlite3/Sqlite3Connection.h"

#include <sqlite3.h>

using namespace monarch::sql;
using namespace monarch::sql::sqlite3;

Sqlite3Exception::Sqlite3Exception(Sqlite3Connection* c) :
   SqlException("", "monarch.sql.sqlite3.Sqlite3")
{
   setCode(sqlite3_errcode(c->getHandle()));
   setMessage(sqlite3_errmsg(c->getHandle()));
}

Sqlite3Exception::~Sqlite3Exception()
{
}
