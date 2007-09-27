/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/database/DatabaseException.h"

#include <string>

using namespace db::database;
using namespace db::rt;

DatabaseException::DatabaseException(const char* message, const char* code) :
   Exception(message, code)
{
   mSqlState = new char[1];
   memset(mSqlState, 0, 1);
}

DatabaseException::~DatabaseException()
{
   delete [] mSqlState;
}

void DatabaseException::setSqlState(const char* state)
{
   delete [] mSqlState;
   mSqlState = new char[strlen(state) + 1];
   strcpy(mSqlState, state);
}

const char* DatabaseException::getSqlState()
{
   return mSqlState;
}
