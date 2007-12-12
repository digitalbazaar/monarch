/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/SqlException.h"

#include <string>

using namespace db::sql;
using namespace db::rt;

SqlException::SqlException(
   const char* message, const char* type, int code) :
   Exception(message, type, code)
{
   mSqlState = strdup("");
}

SqlException::~SqlException()
{
   free(mSqlState);
}

void SqlException::setSqlState(const char* state)
{
   free(mSqlState);
   mSqlState = strdup(state);
}

const char* SqlException::getSqlState()
{
   return mSqlState;
}
