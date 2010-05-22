/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/util/StringTools.h"

#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/util/StringTokenizer.h"
#include "monarch/util/regex/Pattern.h"

#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

using namespace std;
using namespace monarch::rt;
using namespace monarch::util;
using namespace monarch::util::regex;

string& StringTools::trim(string& str, const char* trimChars)
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
   string& str, const char* find, const char* replace,
   string::size_type pos)
{
   string::size_type found = str.find(find, pos);
   if(found != string::npos)
   {
      str.replace(found, strlen(find), replace);
   }
   return str;
}

string& StringTools::replaceAll(
   string& str, const char* find, const char* replace)
{
   int findLen = strlen(find);
   int replaceLen = strlen(replace);
   string::size_type found = str.find(find);
   while(found != string::npos)
   {
      str.replace(found, findLen, replace);
      found = str.find(find, found + replaceLen);
   }
   return str;
}

string& StringTools::regexReplaceAll(
   string& str, const char* regex, const char* replace, bool matchCase)
{
   // compile regex pattern
   PatternRef p = Pattern::compile(regex, matchCase, true);
   if(!p.isNull())
   {
      // replace all matches
      int start, end;
      int index = 0;
      int len = strlen(replace);
      while(p->match(str.c_str(), index, start, end))
      {
         str.replace(start, end - start, replace);
         index = start + len;
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

string StringTools::format(const char* format, ...)
{
   va_list ap;
   va_start(ap, format);
   string rval = vformat(format, ap);
   va_end(ap);
   return rval;
}

DynamicObject StringTools::split(const char* str, const char* delimiter)
{
   DynamicObject rval;
   rval->setType(Array);

   StringTokenizer st(str, delimiter, true);
   while(st.hasNextToken())
   {
      rval->append() = st.nextToken();
   }

   return rval;
}

string StringTools::join(
   DynamicObject dyno, const char* glue, int start, int end)
{
   string rval;

   if(end == -1)
   {
      end = dyno->length();
   }

   int n = 0;
   DynamicObjectIterator i = dyno.getIterator();
   while(i->hasNext())
   {
      if(n >= start && n < end)
      {
         if(rval.length() > 0)
         {
            rval.append(glue);
         }
         rval.append(i->next()->getString());
      }
      else
      {
         i->next();
      }
      n++;
   }

   return rval;
}

string StringTools::join(DynamicObject dyno, const char* glue)
{
   string rval;

   DynamicObjectIterator i = dyno.getIterator();
   while(i->hasNext())
   {
      if(rval.length() > 0)
      {
         rval.append(glue);
      }
      rval.append(i->next()->getString());
   }

   return rval;
}

string StringTools::toUpper(const char* str)
{
   string rval = str;
   std::transform(rval.begin(), rval.end(), rval.begin(), ::toupper);
   return rval;
}

string StringTools::toLower(const char* str)
{
   string rval = str;
   std::transform(rval.begin(), rval.end(), rval.begin(), ::tolower);
   return rval;
}

string StringTools::upperCaseWords(const char* words)
{
   string rval;

   StringTokenizer st;
   st.tokenize(words, ' ');
   while(st.hasNextToken())
   {
      const char* token = st.nextToken();
      if(token[0] != '\0')
      {
         rval.push_back(' ');
         rval.push_back(::toupper(token[0]));
         rval.append(token + 1);
      }
   }

   return rval;
}
