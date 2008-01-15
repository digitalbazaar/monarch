/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_test_Test_H
#define db_test_Test_H

#include "db/rt/Exception.h"
#include "db/util/DynamicObject.h"
#include "db/util/DynamicObjectIterator.h"
#include <ostream>
#include <string>

namespace db
{
namespace test
{

/**
 * Dump exception details.
 */
void dumpException(db::rt::Exception* e);

/**
 * Non-JSON DynamicObject output.
 * Note: Not using defaults on main function due to C++isms.
 *
 * @param dyno DynamicObject to dump.
 * @param doi DynamicObject iterator
 * @param indent indent level
 */
void dumpDynamicObjectText_(
   db::util::DynamicObject& dyno, db::util::DynamicObjectIterator doi = NULL,
   int indent = 0);

/**
 * Non-JSON DynamicObject output.
 *
 * @param dyno DynamicObject to dump.
 */
void dumpDynamicObjectText(db::util::DynamicObject& dyno);

/**
 * Write DynamicObject JSON to an ostream.
 *
 * @param dyno DynamicObject to dump.
 * @param stream stream to write to.
 * @param compact Use compact syntax
 */
void dynamicObjectToStream(
   db::util::DynamicObject& dyno, std::ostream& stream, bool compact = false);

/**
 * Write DynamicObject JSON to a string.
 * Dump DynamicObject details as JSON.
 *
 * @param dyno DynamicObject to dump.
 * @param dyno DynamicObject to dump.
 * @param compact Use compact syntax
 */
void dynamicObjectToString(
   db::util::DynamicObject& dyno, std::string& str, bool compact = false);

/**
 * Dump DynamicObject details as JSON to cout.
 * 
 * @param dyno DynamicObject to dump.
 * @param compact Use compact syntax
 */
void dumpDynamicObject(db::util::DynamicObject& dyno, bool compact = false);

/**
 * Check and dump exception condition.
 */
#define assertNoException() \
   do { \
      if(db::rt::Exception::hasLast()) \
      { \
         db::rt::Exception* e = db::rt::Exception::getLast(); \
         db::test::dumpException(e); \
         assert(!db::rt::Exception::hasLast()); \
      } \
   } while(0)

/**
 * Check exception is set.
 */
#define assertException() \
   do { \
      if(!db::rt::Exception::hasLast()) \
      { \
         db::rt::Exception e("Test expected an Exception but there wasn't one!"); \
         db::test::dumpException(&e); \
         assert(db::rt::Exception::hasLast()); \
      } \
   } while(0)

/**
 * Assert strings are equal.
 */
#define assertStrCmp(a, b) \
   do { \
      if(strcmp(a, b) != 0) \
      { \
         std::cout << \
            "string a='" << a << \
            "',string b='" << b << "'" << std::endl; \
         assert(std::strcmp(a, b) == 0); \
      } \
   } while(0)

} // end namespace test
} // end namespace db

#endif
