/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/StringTools.h"
// FIXME:
//#include "db/util/regex/Pattern.h"

using namespace std;
using namespace db::util;
// FIXME:
//using namespace db::util::regex;

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
//   // compile regex pattern
//   Pattern* p = Pattern::compile(regex.c_str());
//   if(p != NULL)
//   {
//      // replace all matches
//      unsigned int start, end;
//      unsigned int index = 0;
//      while(p->match(str.c_str(), index, start, end))
//      {
//         str.replace(start, end - start, replace);
//         index = start + replace.length();
//      }
//      
//      // free regex pattern
//      delete p;
//   }
   
   return str;
}
