/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/sqlite3/Sqlite3Exception.h"

#include "monarch/sql/sqlite3/Sqlite3Connection.h"

#include <sqlite3.h>

using namespace monarch::rt;
using namespace monarch::sql::sqlite3;

Sqlite3Exception::Sqlite3Exception()
{
}

Sqlite3Exception::~Sqlite3Exception()
{
}

Exception* Sqlite3Exception::create(Sqlite3Connection* c)
{
   return new Exception(
      sqlite3_errmsg(c->getHandle()),
      "monarch.sql.sqlite3.Sqlite3",
      sqlite3_errcode(c->getHandle()));
}
