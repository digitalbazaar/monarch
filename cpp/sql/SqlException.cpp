/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/SqlException.h"

#include "monarch/rt/DynamicObject.h"

#include <string>

using namespace monarch::sql;
using namespace monarch::rt;

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
