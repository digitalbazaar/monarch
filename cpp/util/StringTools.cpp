/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/util/StringTools.h"

#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/StringTokenizer.h"

#include <cerrno>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

using namespace std;
using namespace monarch::rt;
using namespace monarch::util;

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
      regexReplaceAll(str, p, replace);
   }
   return str;
}

string& StringTools::regexReplaceAll(
   string& str, PatternRef& p, const char* replace)
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
   return str;
}

std::string& StringTools::regexRewrite(
   std::string& str, const char* regex,
   const char* replace, bool matchCase,
   bool* matched)
{
   // compile regex pattern
   PatternRef p = Pattern::compile(regex, matchCase, true);
   if(!p.isNull())
   {
      regexRewrite(str, p, replace, matched);
   }
   return str;
}

std::string& StringTools::regexRewrite(
   std::string& str, PatternRef& p, const char* replace, bool* matched)
{
   // get sub matches
   DynamicObject subs;
   if(p->getSubMatches(str.c_str(), subs))
   {
      if(matched != NULL)
      {
         *matched = true;
      }

      // do formatted replacement
      str.erase();
      const char* ptr = replace;
      const char* v;
      do
      {
         v = strpbrk(ptr, "$\\");
         if(v == NULL)
         {
            // append remainder
            str.append(ptr);
         }
         else if(*v == '\\')
         {
            // add everything up to this point
            str.append(ptr, v - ptr);

            ++v;
            switch(*v)
            {
               case '$':
               case '\\':
                  // add escaped '$' or '\'
                  str.push_back(*v);
                  break;
               case 'b':
                  str.push_back('\b');
                  break;
               case 'n':
                  str.push_back('\n');
                  break;
               case 't':
                  str.push_back('\t');
                  break;
               case 'r':
                  str.push_back('\r');
                  break;
               default:
                  // unknown escape sequence, just add it normally
                  str.push_back('\\');
                  str.push_back(*v);
                  break;
            }
            ++v;
         }
         else
         {
            // add everything up to this point
            str.append(ptr, v - ptr);

            // check for a number
            char* end;
            int idx = strtol(++v, &end, 10);
            if(idx < subs->length())
            {
               str.append(subs[idx]->getString());
            }
            v = end;
         }
         ptr = v;
      }
      while(ptr != NULL);
   }
   else if(matched != NULL)
   {
      *matched = false;
   }

   return str;
}

bool StringTools::vsnformat(
   string& str, size_t size, const char *format, va_list varargs)
{
   // Adapted from glibc sprintf docs.

   bool rval;

   int n;
   size_t psize;
   char *p;
   char *np;

   // Guess we need no more than 128 bytes to start.
   psize = 25;
   // limit size if needed
   if(size != 0)
   {
      psize = min(psize, size);
   }

   // allocate initial buffer
   p = (char*)malloc(psize);
   rval = (p != NULL);

   bool done = false;
   while(rval && !done)
   {
      // Try to print in the allocated space.
      n = vsnprintf(p, psize, format, varargs);

      // If that worked, return the string.
      if(n > -1 && (size_t)n < psize)
      {
         str.assign(p);
         free(p);
         done = true;
      }
      else
      {
         // make sure size isn't at the limit
         // this will enable an attempt with at least 1 more byte
         if(size != 0)
         {
            rval = (size > psize);
         }

         if(rval)
         {
            // Else try again with more space.
            if(n > -1)
            {
               // glibc 2.1
               // n is precisely what is needed
               psize = n + 1;
            }
            else
            {
               // glibc 2.0
               // guess new size, but limit increase in size for huge strings
               // FIXME: check psize < (max size_t / 2) to avoid overflow
               psize = min(psize * 2, psize + 16777216 /* 16MiB */);
               // limit size if needed
               if(size != 0)
               {
                  psize = min(psize, size);
               }
            }

            // try to realloc the buffer
            np = (char*)realloc(p, psize);
            rval = (np != NULL);
            if(rval)
            {
               p = np;
            }
         }
      }
   }

   if(!rval)
   {
      if(size == psize)
      {
         // failure due to limited size request
         ExceptionRef e = new Exception(
            "String formatting size overflow error.",
            "monarch.util.StringUtils.SizeOverflowError");
         e->getDetails()["format"] = format;
         e->getDetails()["maxSize"] = (int64_t)size;
         e->getDetails()["attemptedSize"] = (int64_t)psize;
         Exception::push(e);
      }
      else
      {
         // failure due to allocation error
         ExceptionRef e = new Exception(
            "String formating memory allocation error.",
            "monarch.util.StringUtils.MemoryAllocationError");
         e->getDetails()["format"] = format;
         e->getDetails()["errno"] = errno;
         e->getDetails()["maxSize"] = (int64_t)size;
         e->getDetails()["attemptedSize"] = (int64_t)psize;
         Exception::push(e);
      }
   }

   return rval;
}

bool StringTools::vsformat(string& str, const char *format, va_list varargs)
{
   return vsnformat(str, 0, format, varargs);
}

string StringTools::vformat(const char* format, va_list varargs)
{
   string rval;
   if(!vsformat(rval, format, varargs))
   {
      // no real error handling, just clear string
      rval.clear();
   }
   return rval;
}

bool StringTools::snformat(string& str, size_t size, const char *format, ...)
{
   bool rval;

   va_list ap;
   va_start(ap, format);
   rval = vsnformat(str, size, format, ap);
   va_end(ap);

   return rval;
}

bool StringTools::sformat(string& str, const char *format, ...)
{
   bool rval;

   va_list ap;
   va_start(ap, format);
   rval = vsformat(str, format, ap);
   va_end(ap);

   return rval;
}

string StringTools::format(const char* format, ...)
{
   string rval;

   va_list ap;
   va_start(ap, format);
   rval = vformat(format, ap);
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
      rval->append(st.nextToken());
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
      ++n;
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
