/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/sql/Statement.h"

#include <cstdlib>

using namespace db::sql;

Statement::Statement(Connection* c, const char* sql)
{
   mConnection = c;
   mSql = strdup(sql);
}

Statement::~Statement()
{
   free(mSql);
}

inline const char* Statement::getSql()
{
   return mSql;
}

inline Connection* Statement::getConnection()
{
   return mConnection;
}
