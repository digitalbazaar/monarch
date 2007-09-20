/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/database/RowIterator.h"

using namespace db::database;

RowIterator::RowIterator(Statement* s)
{
   mStatement = s;
}

RowIterator::~RowIterator()
{
}
