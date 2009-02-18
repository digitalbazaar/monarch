/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_StringTools_H
#define db_util_StringTools_H

#include <cstring>
#include <string>

namespace db
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
    * 
    * @return a reference to "str".
    */
   static std::string& regexReplaceAll(
      std::string& str, const std::string& regex, const std::string& replace);
   
   /**
    * Creates an std::string from a printf format. 
    * 
    * @param f the format.
    * 
    * @return the std::string.
    */
   static std::string format(const char* f, ...);
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
} // end namespace db
#endif
