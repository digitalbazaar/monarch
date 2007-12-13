/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_test_Test_H
#define db_test_Test_H

#include "db/rt/Exception.h"
#include "db/util/DynamicObject.h"
#include "db/util/DynamicObjectIterator.h"

namespace db
{
namespace test
{

/**
 * Dump exception details.
 */
void dumpException(db::rt::Exception* e);

/**
 * Non-JSON DynamicObject output
 *
 * @param dyno DynamicObject to dump.
 * @param doi DynamicObject iterator
 * @param indent indent level
 */
void dumpDynamicObjectText_(
   db::util::DynamicObject dyno, db::util::DynamicObjectIterator doi = NULL,
   int indent = 0);

/**
 * Non-JSON DynamicObject output
 *
 * @param dyno DynamicObject to dump.
 */
void dumpDynamicObjectText(db::util::DynamicObject dyno);

/**
 * Dump DynamicObject details as JSON.
 * 
 * @param dyno DynamicObject to dump.
 * @param compact Use compact syntax
 */
void dumpDynamicObject_(db::util::DynamicObject dyno, bool compact);

/**
 * Dump DynamicObject details as JSON.
 *
 * @param dyno DynamicObject to dump.
 */
void dumpDynamicObject(db::util::DynamicObject dyno);

/**
 * Check and dump exception condition.
 */
void assertNoException();

/**
 * Check exception is set.
 */
void assertException();

/**
 * Assert strings are equal.
 */
#define assertStrCmp(a, b) assert(strcmp(a, b) == 0)

} // end namespace test
} // end namespace db

#endif
