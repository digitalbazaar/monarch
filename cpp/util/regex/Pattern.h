/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_util_regex_Pattern_H
#define db_util_regex_Pattern_H

#include <regex.h>
#include "db/rt/Collectable.h"

namespace db
{
namespace util
{
namespace regex
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
    * sets the offsets for the next match, if one can be found.
    *
    * @param str the string to match this pattern against.
    * @param offset the offset in the string to start matching at.
    * @param start the starting offset for the found match, if one was found.
    * @param end the ending offset for the found match, if one was found.
    *
    * @return true if a match was found, false if not.
    */
   virtual bool match(
      const char* str, unsigned int offset,
      unsigned int& start, unsigned int& end);

   /**
    * Matches this pattern against the given string.
    *
    * @param str the string to match this pattern against.
    *
    * @return true if the passed string matches the regex, false if not.
    */
   virtual bool match(const char* str);

   /**
    * Compiles a regular expression into a Pattern.
    *
    * @param regex the regular expression to compile.
    * @param matchCase true to match case, false to be case-insensitive.
    * @param subMatches true to allow sub-matches, false not to.
    *
    * @return the compiled Pattern or NULL if an exception occurred.
    */
   static db::rt::Collectable<Pattern> compile(
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
typedef db::rt::Collectable<Pattern> PatternRef;

} // end namespace regex
} // end namespace util
} // end namespace db
#endif
