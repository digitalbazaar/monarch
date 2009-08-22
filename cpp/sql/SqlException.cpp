/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/sql/SqlException.h"

#include "db/rt/DynamicObject.h"

#include <string>

using namespace db::sql;
using namespace db::rt;

SqlException::SqlException(
   const char* message, const char* type, int code) :
   Exception(message, type, code)
{
}

SqlException::~SqlException()
{
}

inline void SqlException::setSqlState(const char* state)
{
   getDetails()["sqlState"] = state;
}

inline const char* SqlException::getSqlState()
{
   return getDetails()["sqlState"]->getString();
}
