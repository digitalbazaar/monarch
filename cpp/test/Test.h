/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_test_Test_H
#define monarch_test_Test_H

#include "monarch/rt/Exception.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/util/Macros.h"
#include <string>
#include <cassert>

namespace monarch
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
 * Check if exception set. If set, dump exception and fail via assert().
 */
#define assertNoExceptionSet() \
   MO_STMT_START { \
      if(monarch::rt::Exception::isSet()) \
      { \
         monarch::rt::ExceptionRef e = monarch::rt::Exception::get(); \
         monarch::test::dumpException(e); \
         assert(!monarch::rt::Exception::isSet()); \
      } \
   } MO_STMT_END

/**
 * Check if expression is true and if it set any exceptions. If exception set,
 * dump exception and fail via assert. If expression false, fail via assert.
 */
#define assertNoException(expr) \
   MO_STMT_START { \
      bool _success = (expr); \
      assertNoExceptionSet(); \
      assert(_success); \
   } MO_STMT_END

/**
 * Check if exception set. If not set, fail via assert.
 */
#define assertExceptionSet() \
   MO_STMT_START { \
      if(!monarch::rt::Exception::isSet()) \
      { \
         monarch::rt::ExceptionRef e = \
            new monarch::rt::Exception( \
               "Test expected an Exception but there wasn't one."); \
         monarch::test::dumpException(e); \
         assert(monarch::rt::Exception::isSet()); \
      } \
   } MO_STMT_END

/**
 * Check if expression is false and if it set any exceptions. If exception not
 * set, fail via assert. If expression true, fail via assert.
 */
#define assertException(expr) \
   MO_STMT_START { \
      bool _success = (expr); \
      assertExceptionSet(); \
      assert(!_success); \
   } MO_STMT_END

/**
 * Assert named strings are equal.
 *
 * @param name1 name of first string
 * @param str1 first string
 * @param name2 name of second string
 * @param str2 second string
 */
#define assertNamedStrCmp(name1, str1, name2, str2) \
   MO_STMT_START { \
      std::string _str1 = str1; \
      std::string _str2 = str2; \
      if(strcmp(_str1.c_str(), _str2.c_str()) != 0) \
      { \
         printf("\n%s:\n'%s'\n%s:\n'%s'\n", \
            name1, _str1.c_str(), name2, _str2.c_str()); \
         assert(std::strcmp(_str1.c_str(), _str2.c_str()) == 0); \
      } \
   } MO_STMT_END

/**
 * Assert strings are equal.
 *
 * @param str1 first string
 * @param str2 second string
 */
#define assertStrCmp(str1, str2) \
   assertNamedStrCmp("string a", str1, "string b", str2)

/**
 * Assert named DynamicObjects are equal.
 *
 * @param name1 name of first DynamicObject
 * @param dyno1 first DynamicObject
 * @param name2 name of second DynamicObject
 * @param dyno2 second DynamicObject
 */
#define assertNamedDynoCmp(name1, dyno1, name2, dyno2) \
   MO_STMT_START { \
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
   } MO_STMT_END

/**
 * Assert DynamicObjects are equal.
 *
 * @param dyno1 first DynamicObject
 * @param dyno2 second DynamicObject
 */
#define assertDynoCmp(dyno1, dyno2) \
   assertNamedDynoCmp("dyno a", dyno1, "dyno b", dyno2)

} // end namespace test
} // end namespace monarch

#endif
