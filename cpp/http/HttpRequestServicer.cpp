/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpRequestServicer.h"

#include <cstdlib>
#include <cstring>

using namespace monarch::http;

HttpRequestServicer::HttpRequestServicer(const char* path, bool isRegex)
{
   mPath = (char*)malloc(strlen(path) + 2);
   mPathIsRegex = isRegex;
   normalizePath(path, mPath);
}

HttpRequestServicer::~HttpRequestServicer()
{
   // free path
   free(mPath);
}

inline const char* HttpRequestServicer::getPath()
{
   return mPath;
}

inline bool HttpRequestServicer::isPathRegex()
{
   return mPathIsRegex;
}

void HttpRequestServicer::normalizePath(
   const char* inPath, char* outPath, bool pathIsRegex)
{
   // prepend slash if as necessary
   int i = 0;
   int inLength = strlen(inPath);

   // if the path doesn't start with a '/', or the path is a regex and
   // doesn't start with "\/", then prepend a '/' character to the path
   bool prependSlash = true; 
   if(inPath[0] == '/')
   {
      prependSlash = false;
   }
   if(pathIsRegex && inLength > 1 && inPath[0] == '\\' && inPath[1] == '/')
   {
      prependSlash = false;
   }
   if(prependSlash)
   {
      outPath[i++] = '/';
   }

   // copy strings, removing duplicate slashes before the query
   bool query = false;
   for(int n = 1; inPath[n - 1] != 0; ++n)
   {
      bool addedCharacter = false;

      // if the current path is not a regex and a '?' character is found, mark
      // this as the query boundary. If the path is a regex and a "\?" is
      // detected, mark it as the query boundary
      if((!pathIsRegex && (inPath[n - 1] == '?')) ||
         (pathIsRegex && n > 1 && (inPath[n - 2] == '\\') && 
            (inPath[n - 1] == '?')))
      {
         query = true;
      }

      if(query)
      {
         outPath[i++] = inPath[n - 1];
         addedCharacter = true;
      }
      // if the current path contains a "//", reduce it to a single '/'. 
      else if(inPath[n] != '/' || inPath[n - 1] != '/')
      {
         outPath[i++] = inPath[n - 1];
         addedCharacter = true;
      }

      // if the current path is a regex and contains "\/\/" or "/\/"
      // reduce it to a single "\/" or "/".
      if(pathIsRegex && n > 3 && 
         ((inPath[n] == '/' &&  inPath[n - 1] == '\\' && 
         inPath[n - 2] == '/') ||
         (inPath[n] == '/' &&  inPath[n - 1] == '\\' && 
         inPath[n - 2] == '/' && inPath[n - 3] == '\\')))
      {
         i -= 2;
      }
   }
   outPath[i] = 0;

   // ensure path doesn't end in slash. If the current path is a regex, make
   // sure the path doesn't end in "\/"
   // FIXME: Why search for a '&' here - '&' can't come before query 
   //        parameter, can it? '&' is a subdelimiter per the URI spec. -- manu
   int slash = strcspn(outPath, "?&") - 1;
   int negativeOffset = 0;
   bool shift = false;
   if(!pathIsRegex && slash > 0 && outPath[slash] == '/')
   {
      shift = true;
   }
   if(pathIsRegex)
   {
      // FIXME: Not searching for '&' like above, this code may be incorrect.
      char* slashPtr = strstr(outPath, "\\?");
      slash = strlen(outPath) - 1;
      
      if(slashPtr != NULL)
      {
         slash = (unsigned int)slashPtr - (unsigned int)outPath - 1;
      }
      if(slash > 0 && (outPath[slash] == '/') && (outPath[slash - 1] != '\\'))
      {
         shift = true;
      }
      else if(slash > 0 && 
         (outPath[slash] == '/') && (outPath[slash - 1] == '\\'))
      {
         shift = true;
         negativeOffset = 1;
      }
   }

   // shift left to remove the trailing slash that was detected
   if(shift)
   {
      for(; outPath[slash] != 0; ++slash)
      {
         outPath[slash - negativeOffset] = outPath[slash + 1];
      }
   }
}

