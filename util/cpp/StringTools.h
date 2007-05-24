/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef StringTools_H
#define StringTools_H

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
};

} // end namespace util
} // end namespace db
#endif
