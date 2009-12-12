/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/sql/Row.h"

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
