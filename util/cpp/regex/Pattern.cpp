/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Pattern.h"

#include <string>

using namespace std;
using namespace db::util::regex;

Pattern::Pattern()
{
}

Pattern::~Pattern()
{
   // free storage
   regfree(&getStorage());
}

regex_t& Pattern::getStorage()
{
   return mStorage;
}

bool Pattern::match(
   const char* str, unsigned int offset,
   unsigned int& start, unsigned int& end)
{
   bool rval = false;
   
   // create match struct
   regmatch_t match[1];
   
   // execute regex
   if(regexec(&getStorage(), str + offset, 1, match, 0) == 0)
   {
      rval = true;
      
      // set start and end offsets
      start = match[0].rm_so + offset;
      end = match[0].rm_eo + offset;
   }
   
   return rval;
}

Pattern* Pattern::compile(const char* regex, bool matchCase, bool subMatches)
throw(InvalidRegexException)
{
   // create a new Pattern
   Pattern* p = new Pattern();
   
   // use extended regular expressions
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
      
      // create message for exception
      string message = "Invalid regular expression! ";
      message += str;
      
      // delete pattern upon compilation failure
      delete p;
      p = NULL;
      
      // throw exception
      throw InvalidRegexException(message);
   }
   
   return p;
}

bool Pattern::match(const char* regex, const char* str, bool matchCase)
throw(InvalidRegexException)
{
   bool rval = false;
   
   // compile a pattern with no sub matches allowed
   Pattern* p = compile(regex, matchCase, false);
   
   // execute regex
   rval = regexec(&p->getStorage(), str, 0, NULL, 0) == 0;
   
   // delete pattern
   delete p;
   
   return rval;
}
