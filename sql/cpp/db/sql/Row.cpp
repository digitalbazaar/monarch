/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
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

Statement* Row::getStatement()
{
   return mStatement;
}
