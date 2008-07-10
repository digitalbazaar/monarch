/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/Row.h"

using namespace db::sql;

Row::Row(Statement* s)
{
   mStatement = s;
}

Row::~Row()
{
}

inline Statement* Row::getStatement()
{
   return mStatement;
}
