/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_util_StringTools_H
#define monarch_util_StringTools_H

#include "monarch/rt/DynamicObject.h"
#include <cstring>
#include <string>

namespace monarch
{
namespace util
{

/**
 * The StringTools class provides methods for manipulating strings.
 *
 * @author Dave Longley
 */
class StringTools
{
private:
   /**
    * Creates a new StringTools object.
    */
   StringTools() {};

public:
   /**
    * Trims characters from the front and back of the passed string. Any
    * character in the second parameter will be trimmed.
    *
    * @param str the string to trim the characters from.
    * @param trimChars the trim characters (defaults to a single space).
    *
    * @return a reference to the trimmed string.
    */
   static std::string& trim(
      std::string& str, const std::string& trimChars = " ");

   /**
    * Replaces the first instance in "str" starting at "pos" of "find" with
    * "replace".
    *
    * @param str the string to operate on.
    * @param find the string to find.
    * @param replace the string to replace "find" with.
    * @param pos the position to start searching at.
    *
    * @return a reference to "str".
    */
   static std::string& replace(
      std::string& str, const std::string& find, const std::string& replace,
      std::string::size_type pos = 0);

   /**
    * Replaces all instances in "str" of "find" with "replace".
    *
    * @param str the string to operate on.
    * @param find the string to find.
    * @param replace the string to replace "find" with.
    *
    * @return a reference to "str".
    */
   static std::string& replaceAll(
      std::string& str, const std::string& find, const std::string& replace);

   /**
    * Replaces all instances in "str" that match "regex" with "replace".
    *
    * @param str the string to operate on.
    * @param regex the regular expression to match.
    * @param replace the string to replace "find" with.
    * @param matchCase true to match case, false for case-insensitive.
    *
    * @return a reference to "str".
    */
   static std::string& regexReplaceAll(
      std::string& str, const std::string& regex,
      const std::string& replace, bool matchCase = true);

   /**
    * Creates an std::string from a printf format.
    *
    * @param f the format.
    *
    * @return the std::string.
    */
   static std::string format(const char* format, ...)
#ifdef __GNUC__
      __attribute__ ((format (printf, 1, 2)))
#endif
         ;

   /**
    * Splits a string into a DynamicObject array using the given character
    * delimiter to split on.
    *
    * @param str the string to split.
    * @param delimeter the string to split on.
    *
    * @return the DynamicObject array.
    */
   static monarch::rt::DynamicObject split(
      const char* str, const char* delimiter);

   /**
    * Joins a DynamicObject array or map values using the given string as
    * the glue between elements.
    *
    * @param dyno the DynamicObject array or map.
    * @param glue the string to use as glue.
    * @param start the DynamicObject starting index (inclusive).
    * @param end the DynamicObject ending index (exclusive), -1 for length.
    *
    * @return the joined string.
    */
   static std::string join(
      monarch::rt::DynamicObject dyno, const char* glue,
      int start, int end = -1);

   /**
    * Joins DynamicObject array or map values using the given string as
    * the glue between elements.
    *
    * @param dyno the DynamicObject array or map.
    * @param glue the string to use as glue.
    *
    * @return the joined string.
    */
   static std::string join(monarch::rt::DynamicObject dyno, const char* glue);

   /**
    * Converts the passed string to upper case.
    *
    * @param str the string to convert.
    *
    * @return the result.
    */
   static std::string toUpper(const char* str);

   /**
    * Converts the passed string to lower case.
    *
    * @param str the string to convert.
    *
    * @return the result.
    */
   static std::string toLower(const char* str);
};

/**
 * A StringComparator compares two strings.
 */
struct StringComparator
{
   /**
    * Compares two null-terminated strings, returning true if the first is
    * less than the second, false if not. The compare is case-sensitive.
    *
    * @param s1 the first string.
    * @param s2 the second string.
    *
    * @return true if the s1 < s2, false if not.
    */
   bool operator()(const char* s1, const char* s2) const
   {
      return strcmp(s1, s2) < 0;
   }
};

/**
 * A StringComparator compares two strings using a case-insensitive compare.
 */
struct StringCaseComparator
{
   /**
    * Compares two null-terminated strings, returning true if the first is
    * less than the second, false if not. The compare is case-insensitive.
    *
    * @param s1 the first string.
    * @param s2 the second string.
    *
    * @return true if the s1 < s2, false if not.
    */
   bool operator()(const char* s1, const char* s2) const
   {
      return strcasecmp(s1, s2) < 0;
   }
};

} // end namespace util
} // end namespace monarch
#endif
