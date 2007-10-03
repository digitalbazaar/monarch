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
   mSqlState = new char[1];
   memset(mSqlState, 0, 1);
}

SqlException::~SqlException()
{
   delete [] mSqlState;
}

void SqlException::setSqlState(const char* state)
{
   delete [] mSqlState;
   mSqlState = new char[strlen(state) + 1];
   strcpy(mSqlState, state);
}

const char* SqlException::getSqlState()
{
   return mSqlState;
}
