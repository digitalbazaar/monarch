/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/util/StringTools.h"

#include "db/util/regex/Pattern.h"

#include <cstring>
#include <cstdarg>
#include <cstdlib>

using namespace std;
using namespace db::util;
using namespace db::util::regex;

string& StringTools::trim(string& str, const string& trimChars)
{
   // erase front trim characters
   str.erase(0, str.find_first_not_of(trimChars));

   // find back trim characters
   string::size_type last = str.find_last_not_of(trimChars);
   if(last != string::npos)
   {
      // erase back trim characters
      str.erase(last + 1);
   }

   return str;
}

string& StringTools::replace(
   string& str, const string& find, const string& replace,
   string::size_type pos)
{
   string::size_type found = str.find(find, pos);
   if(found != string::npos)
   {
      str.replace(found, find.length(), replace);
   }

   return str;
}

string& StringTools::replaceAll(
   string& str, const string& find, const string& replace)
{
   string::size_type found = str.find(find);
   while(found != string::npos)
   {
      str.replace(found, find.length(), replace);
      found = str.find(find, found + replace.length());
   }

   return str;
}

string& StringTools::regexReplaceAll(
   string& str, const string& regex, const string& replace)
{
   // compile regex pattern
   PatternRef p = Pattern::compile(regex.c_str());
   if(!p.isNull())
   {
      // replace all matches
      unsigned int start, end;
      unsigned int index = 0;
      while(p->match(str.c_str(), index, start, end))
      {
         str.replace(start, end - start, replace);
         index = start + replace.length();
      }
   }

   return str;
}

/**
 * A helper function for StringTools::format().
 *
 * @param f the format.
 * @param ap the variable args list.
 */
static string vformat(const char* f, va_list ap)
{
   // estimate size for string
   int size = 256;
   char* str = (char*)malloc(size);

   // copy va_list in case we must realloc
   va_list clone;
   va_copy(clone, ap);

   // try to get formatted string
   size = vsnprintf(str, size, f, ap);

   // if size > 0, then string was truncated and size contains
   // full size of formatted string (not including null-terminator)
   if(size > 0)
   {
      // include room for null-terminator
      size++;
      str = (char*)realloc(str, size);
      vsnprintf(str, size, f, ap);
   }

   string rval = str;
   free(str);
   return rval;
}

string StringTools::format(const char* f, ...)
{
   va_list ap;
   va_start(ap, f);
   string rval = vformat(f, ap);
   va_end(ap);
   return rval;
}
