/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_util_Pattern_H
#define monarch_util_Pattern_H

#include <pcreposix.h>
#include "monarch/rt/DynamicObject.h"

namespace monarch
{
namespace util
{

/**
 * A Pattern is a utility class for storing compiled regular expressions.
 *
 * @author Dave Longley
 */
class Pattern
{
protected:
   /**
    * The storage struct for the pattern.
    */
   regex_t mStorage;

   /**
    * Creates a new Pattern.
    */
   Pattern();

   /**
    * Gets the storage for this Pattern.
    *
    * @return the storage for this Pattern.
    */
   regex_t& getStorage();

public:
   /**
    * Destructs this Pattern.
    */
   virtual ~Pattern();

   /**
    * Matches this pattern against the passed string at the given offset and
    * sets the offsets for the next match, if one can be found. The pattern
    * must have been compiled with submatch support enabled.
    *
    * @param str the string to match this pattern against.
    * @param offset the offset in the string to start matching at.
    * @param start the starting offset for the found match, if one was found.
    * @param end the ending offset for the found match, if one was found.
    *
    * @return true if a match was found, false if not.
    */
   virtual bool match(const char* str, int offset, int& start, int& end);

   /**
    * Matches this pattern against the given string.
    *
    * @param str the string to match this pattern against.
    *
    * @return true if the passed string matches the regex, false if not.
    */
   virtual bool match(const char* str);

   /**
    * Matches this pattern against the given string and returns the portions
    * of the string that matched subexpressions in the pattern. The pattern
    * must have been compiled with submatch support enabled.
    *
    * If the given number of subexpressions to retrieve is higher than the
    * number in this pattern then fewer than 'n' results will be in the array.
    *
    * If the given number of subexpressions to retrieve is lower than the
    * number in the pattern, only that given number will be returned per
    * use of the pattern.
    *
    * A pattern that does not start with "^" and end with "$" may be
    * consecutively repeated multiple times to the given string.
    *
    * The full match for each repeat of the pattern can be returned as the
    * first submatch for each repeat. For instance, a pattern that matches
    * "f(.{1})o" in the string "faofbo" will result in an array:
    * ["fao","a","fbo","b"]. If full matches are turned off, then the array
    * will contain: ["a","b"].
    *
    * The number of times the pattern is repeated can be limited using the
    * 'repeats' parameter, where a value of 0 or less indicates no limit.
    *
    * @param str the string to match this pattern against.
    * @param matches the array to store the matches in.
    * @param n the number of subexpressions to get matches for, -1 for all.
    * @param includeFull true to include full matches for each repeat of the
    *           pattern, false not to.
    * @param repeats the number of times to repeat the pattern, 0 for no limit.
    *
    * @return true if the passed string matches the regex, false if not.
    */
   virtual bool getSubMatches(
      const char* str, monarch::rt::DynamicObject& matches,
      int n = -1, bool includeFullMatches = true, int repeats = 0);

   /**
    * An alias for getSubMatches() that will split the given string into the
    * groups specified by the regex. All subexpressions will be used and no
    * full matches will be included.
    *
    * @param str the string to match this pattern against.
    * @param matches the array to store the string matches in, with the first
    *           always containing the full match.
    * @param limit limits the number of times to repeat the regex on the given
    *           string (different from the number of matches to return), 0
    *           indicates no limit.
    *
    * @return true if the passed string matches the regex, false if not.
    */
   virtual bool split(
      const char* str, monarch::rt::DynamicObject& matches, int limit = 0);

   /**
    * Compiles a regular expression into a Pattern.
    *
    * @param regex the regular expression to compile.
    * @param matchCase true to match case, false to be case-insensitive.
    * @param subMatches true to allow sub-matches, false not to.
    *
    * @return the compiled Pattern or NULL if an exception occurred.
    */
   static monarch::rt::Collectable<Pattern> compile(
      const char* regex, bool matchCase = true, bool subMatches = true);

   /**
    * Matches the passed regex against the given string.
    *
    * @param regex the regex to use.
    * @param str the string to match this pattern against.
    * @param matchCase true to match case, false to be case-insensitive.
    *
    * @return true if the passed string matches the regex, false if not or
    *         if an exception occurred.
    */
   static bool match(const char* regex, const char* str, bool matchCase = true);
};

// typedef for a reference counted Pattern class
typedef monarch::rt::Collectable<Pattern> PatternRef;

} // end namespace util
} // end namespace monarch
#endif
