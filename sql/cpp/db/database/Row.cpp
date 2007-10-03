/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/database/Row.h"

using namespace db::database;

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
