/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/util/Pattern.h"

#include <string>

#include "monarch/rt/Exception.h"

using namespace std;
using namespace monarch::rt;
using namespace monarch::util;

Pattern::Pattern()
{
}

Pattern::~Pattern()
{
   // free storage
   regfree(&getStorage());
}

inline regex_t& Pattern::getStorage()
{
   return mStorage;
}

bool Pattern::match(const char* str, int offset, int& start, int& end)
{
   bool rval = false;

   // create match struct
   regmatch_t match[1];
   match[0].rm_so = 0;
   match[0].rm_eo = 0;

   // if offset is not 0, then not at beginning of the line
   int flags = (offset == 0) ? 0 : REG_NOTBOL;

   // execute regex
   if(regexec(&mStorage, str + offset, 1, match, flags) == 0)
   {
      rval = true;

      // set start and end offsets
      start = match[0].rm_so + offset;
      end = match[0].rm_eo + offset;
   }

   return rval;
}

bool Pattern::match(const char* str)
{
   // execute regex
   return (regexec(&mStorage, str, 0, NULL, 0) == 0);
}

bool Pattern::getSubMatches(
   const char* str, DynamicObject& matches, int n,
   bool includeFullMatches, int repeats)
{
   bool rval = false;

   // setup matches, get at least 1 (first match is the whole expression, other
   // subsequent indexes are used for subexpressions)
   matches->setType(Array);
   matches->clear();

   // use all submatches if n < 0, otherwise use specified amount
   // add 1 for full submatch (always present even if we don't include it in
   // the output)
   n = (n <= 0 || mStorage.re_nsub < (unsigned int)n) ?
      mStorage.re_nsub + 1 : n + 1;

   // create match struct
   regmatch_t* m = (regmatch_t*)calloc(n, sizeof(regmatch_t));

   // keep executing regex on string until it doesn't match
   int count = 0;
   while((repeats <= 0 || count < repeats) &&
         regexec(&mStorage, str, n, m, 0) == 0)
   {
      rval = true;

      // store all subexpression matches (including full match if appropriate)
      int i = (includeFullMatches ? 0 : 1);
      int start, end;
      for(; i < n && m[i].rm_so != -1; ++i)
      {
         // get start and end offsets
         start = m[i].rm_so;
         end = m[i].rm_eo;
         matches->append() = string(str + start, end - start).c_str();
      }

      /* Note: Advancing the string pointer forward (to repeat the pattern) is
       a little bit wonky because you don't know how many characters in the
       string were non-matching characters. This means they might be examined
       again when the user may have been expecting them to have been skipped
       over instead. This is a limitation of the regex api used here -- this
       information is not available to advance the string in a (perhaps) more
       expected fashion.

       For instance, matching "([^,]*)" (looking for things that aren't commas
       in a list of commas) against "ABC,DEF" results in ["ABC","","DEF",""].
       This is because after the first match ("ABC") the string will point at
       the "," following "ABC" instead of at "DEF". The regex api doesn't
       provide the information necessary to advance the string pointer past
       the comma. Hopefully a regex won't be used for an operation such as
       this anyway, but there might be more appropriate uses that produce
       unexpected results.
      */

      // move string pointer past full match to repeat pattern
      if(m[0].rm_eo != 0)
      {
         str += m[0].rm_eo;
         ++count;
      }
      else if(str[0] != '\0')
      {
         // skip current character since match was empty and string isn't
         ++str;
         ++count;
      }
      else
      {
         // full match is empty and so is string, break out
         break;
      }
   }

   free(m);

   return rval;
}

inline bool Pattern::split(
   const char* str, monarch::rt::DynamicObject& matches, int limit)
{
   return getSubMatches(str, matches, -1, false, limit);
}

PatternRef Pattern::compile(const char* regex, bool matchCase, bool subMatches)
{
   // create a new Pattern
   PatternRef p = new Pattern();

   int flags = REG_EXTENDED;

   // use case-insensitive as appropriate
   flags = (matchCase) ? flags : flags | REG_ICASE;

   // use no sub-matches as appropriate
   flags = (subMatches) ? flags : flags | REG_NOSUB;

   // compile the pattern
   int error = regcomp(&p->getStorage(), regex, flags);
   if(error != 0)
   {
      // get a string error (get size first)
      size_t size = regerror(error, &p->getStorage(), NULL, 0);

      char str[size];
      memset(str, '\0', size);
      regerror(error, &p->getStorage(), str, size);

      // clean up pattern upon compilation failure
      p.setNull();

      ExceptionRef e = new Exception(
         "Invalid regular expression.", "monarch.util.InvalidRegex");
      e->getDetails()["error"] = str;
      Exception::set(e);
   }

   return p;
}

bool Pattern::match(const char* regex, const char* str, bool matchCase)
{
   bool rval = false;

   // compile a pattern with no sub matches allowed
   PatternRef p = compile(regex, matchCase, false);

   if(!p.isNull())
   {
      // execute regex
      rval = p->match(str);
   }

   return rval;
}
