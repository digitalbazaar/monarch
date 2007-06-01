/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef StringTools_H
#define StringTools_H

#include <string>
#include "regex/InvalidRegexException.h"

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
    * 
    * @exception InvalidRegexException thrown if the passed regex is invalid.
    */
   static std::string& regexReplaceAll(
      std::string& str, const std::string& regex, const std::string& replace)
   throw(db::util::regex::InvalidRegexException);
};

} // end namespace util
} // end namespace db
#endif
