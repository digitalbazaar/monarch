/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_test_Test_H
#define db_test_Test_H

#include "db/rt/Exception.h"

namespace db
{
namespace test
{

/**
 * Dump exception details.
 */
void dumpException(db::rt::Exception* e);

/**
 * Check and dump exception condition.
 */
void assertNoException();

/**
 * Check exception is set.
 */
void assertException();

} // end namespace test
} // end namespace db

#endif
