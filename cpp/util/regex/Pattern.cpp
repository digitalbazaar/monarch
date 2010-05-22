/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/util/regex/Pattern.h"

#include <string>

#include "monarch/rt/Exception.h"

using namespace std;
using namespace monarch::rt;
using namespace monarch::util::regex;

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
   if(regexec(&getStorage(), str + offset, 1, match, flags) == 0)
   {
      rval = true;

      // set start and end offsets
      start = match[0].rm_so + offset;
      end = match[0].rm_eo + offset;

      // empty string can be returned when submatches is off
      // so match to the end of the string
      if(start == end && offset == 0)
      {
         end = strlen(str);
      }
   }

   return rval;
}

bool Pattern::match(const char* str)
{
   // execute regex
   return (regexec(&mStorage, str, 0, NULL, 0) == 0);
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
