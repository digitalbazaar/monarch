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
   if(regexec(&getStorage(), str + offset, 1, match, flags) == 0)
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

bool Pattern::getSubMatches(const char* str, DynamicObject& matches, int n)
{
   bool rval = false;

   // setup matches, get at least 1 (first match is the whole expression, other
   // subsequent indexes are used for subexpressions)
   matches->setType(Array);
   matches->clear();

   // use all submatches
   // make room for full match
   n = (n < 0) ? getStorage().re_nsub + 1: n + 1;

   // create match struct
   regmatch_t* m = (regmatch_t*)calloc(n, sizeof(regmatch_t));

   // execute regex
   if(regexec(&getStorage(), str, n, m, 0) == 0)
   {
      rval = true;

      // store all subexpression matches (including overall match)
      int start, end;
      for(int i = 0; i < n && m[i].rm_so != -1; i++)
      {
         // get start and end offsets
         start = m[i].rm_so;
         end = m[i].rm_eo;
         matches->append() = string(str + start, end - start).c_str();
      }
   }

   free(m);

   return rval;
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
