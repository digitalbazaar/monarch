/*
 * Copyright (c) 2008-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/util/PathFormatter.h"

#include <cstring>

using namespace std;
using namespace monarch::util;

string& PathFormatter::formatFilename(string& str)
{
   string correctedString;

   // Go through the entire string and clean up any character that is not
   // a universally accepted filename character.
   string::iterator i;
   for(i = str.begin(); i != str.end(); ++i)
   {
      char c = *i;

      // This list was taken from http://en.wikipedia.org/wiki/Filename, we
      // care most about Windows, Linux and Mac OS X. By default all
      // ASCII characters are allowed except for characters that are
      // disallowed on any one of the previously mentioned systems.
      if((c >= ' ') && (c <= '~') && strchr("?/\\*:|\"<>+[]", c) == NULL)
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
