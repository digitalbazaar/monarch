/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/Statement.h"

#include <cstdlib>

using namespace monarch::sql;

Statement::Statement(const char* sql) :
   mSql(strdup(sql))
{
}

Statement::~Statement()
{
   free(mSql);
}

inline const char* Statement::getSql()
{
   return mSql;
}
