/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_test_Test_H
#define db_test_Test_H

#include "monarch/rt/Exception.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include <string>
#include <cassert>

namespace db
{
namespace test
{

/**
 * Dump exception details.
 *
 * @return true on success, false and exception on failure.
 */
bool dumpException(monarch::rt::ExceptionRef& e);

/**
 * Dump exception details of current exception if present.
 *
 * @return true on success, false and exception on failure.
 */
bool dumpException();

/**
 * Non-JSON DynamicObject output.
 * Note: Not using defaults on main function due to C++isms.
 *
 * @param dyno DynamicObject to dump.
 * @param doi DynamicObject iterator
 * @param indent indent level
 */
void dumpDynamicObjectText_(
   monarch::rt::DynamicObject& dyno, monarch::rt::DynamicObjectIterator doi = NULL,
   int indent = 0);

/**
 * Non-JSON DynamicObject output.
 *
 * @param dyno DynamicObject to dump.
 */
void dumpDynamicObjectText(monarch::rt::DynamicObject& dyno);

/**
 * Write DynamicObject JSON to an ostream.
 *
 * @param dyno DynamicObject to dump.
 * @param stream stream to write to.
 * @param compact Use compact syntax
 *
 * @return true on success, false and exception on failure.
 */
bool dynamicObjectToOStream(
   monarch::rt::DynamicObject& dyno, std::ostream& stream, bool compact = false);

/**
 * Write DynamicObject JSON to a string.
 * Dump DynamicObject details as JSON.
 *
 * @param dyno DynamicObject to dump.
 * @param str string to write to.
 * @param compact Use compact syntax
 *
 * @return true on success, false and exception on failure.
 */
bool dynamicObjectToString(
   monarch::rt::DynamicObject& dyno, std::string& str, bool compact = false);

/**
 * Dump DynamicObject details as JSON to cout.
 *
 * @param dyno DynamicObject to dump.
 * @param compact Use compact syntax
 *
 * @return true on success, false and exception on failure.
 */
bool dumpDynamicObject(monarch::rt::DynamicObject& dyno, bool compact = false);

/**
 * Check and dump exception condition.
 */
#define assertNoException() \
   do { \
      if(monarch::rt::Exception::isSet()) \
      { \
         monarch::rt::ExceptionRef e = monarch::rt::Exception::get(); \
         monarch::test::dumpException(e); \
         assert(!monarch::rt::Exception::isSet()); \
      } \
   } while(0)

/**
 * Check exception is set.
 */
#define assertException() \
   do { \
      if(!monarch::rt::Exception::isSet()) \
      { \
         monarch::rt::ExceptionRef e = \
            new monarch::rt::Exception( \
               "Test expected an Exception but there wasn't one!"); \
         monarch::test::dumpException(e); \
         assert(monarch::rt::Exception::isSet()); \
      } \
   } while(0)

/**
 * Assert strings are equal.
 */
#define assertStrCmp(a, b) \
   do { \
      if(strcmp(a, b) != 0) \
      { \
         printf("\nstring a=\n'%s'\nstring b=\n'%s'\n", a, b); \
         assert(std::strcmp(a, b) == 0); \
      } \
   } while(0)

/**
 * Assert named DynamicObjects are equal.
 *
 * @param name1 name of first DynamicObject
 * @param dyno1 first DynamicObject
 * @param name2 name of second DynamicObject
 * @param dyno2 second DynamicObject
 */
#define assertNamedDynoCmp(name1, dyno1, name2, dyno2) \
   do { \
      if(!(dyno1 == dyno2)) \
      { \
         printf("\n%s:\n", name1); \
         monarch::data::json::JsonWriter::writeToStdOut( \
            dyno1, false, false); \
         printf("%s:\n", name2); \
         monarch::data::json::JsonWriter::writeToStdOut( \
            dyno2, false, false); \
         printf("Difference:\n"); \
         monarch::rt::DynamicObject diff; \
         dyno1.diff(dyno2, diff); \
         monarch::data::json::JsonWriter::writeToStdOut( \
            diff, false, false); \
         assert(dyno1 == dyno2); \
      } \
   } while(0)

/**
 * Assert DynamicObjects are equal.
 *
 * @param dyno1 first DynamicObject
 * @param dyno2 second DynamicObject
 */
#define assertDynoCmp(dyno1, dyno2) \
   assertNamedDynoCmp("dyno a", dyno1, "dyno b", dyno2)

} // end namespace test
} // end namespace db

#endif
