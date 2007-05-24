/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "StringTools.h"

using namespace std;
using namespace db::util;

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
      found = str.find(find);
   }
   
   return str;
}
