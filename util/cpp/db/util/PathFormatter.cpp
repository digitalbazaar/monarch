/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/PathFormatter.h"

using namespace std;
using namespace db::util;

string& PathFormatter::formatFilename(string& str)
{   
   string correctedString;
   
   // Go through the entire string and clean up any character that is not
   // a universally accepted filename character.
   string::iterator i;
   for(i = str.begin(); i != str.end(); i++ ) 
   {
      char c = *i;
      
      // this list was taken from http://en.wikipedia.org/wiki/Filename, we
      // care most about Windows, Linux and Mac OS X. If a character is 
      // disallowed on any one of those systems, they're disallowed from all
      // of them.
      if((c >= ' ') && (c <= '~') && (c != '?') && (c != '/') && (c != '\\') &&
         (c != '*') && (c != ':') && (c != '|') && (c != '"') && (c != '<') &&
         (c != '>') && (c != '+') && (c != '[') && (c != ']'))
      {
         correctedString.push_back(c);
      }
      else
      {
         correctedString.push_back('_');
      }
   } 
   
   str = correctedString;
   
   return str;
}
