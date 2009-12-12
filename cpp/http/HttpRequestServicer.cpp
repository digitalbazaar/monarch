/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpRequestServicer.h"

#include <cstdlib>
#include <cstring>

using namespace monarch::http;

HttpRequestServicer::HttpRequestServicer(const char* path)
{
   mPath = (char*)malloc(strlen(path) + 2);
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

void HttpRequestServicer::normalizePath(const char* inPath, char* outPath)
{
   // prepend slash if as necessary
   int i = 0;
   if(inPath[0] != '/')
   {
      outPath[i++] = '/';
   }

   // copy strings, removing duplicate slashes before the query
   bool query = false;
   for(int n = 1; inPath[n - 1] != 0; n++)
   {
      if(inPath[n - 1] == '?')
      {
         query = true;
      }

      if(query)
      {
         outPath[i++] = inPath[n - 1];
      }
      else if((inPath[n] != '/' || inPath[n - 1] != '/'))
      {
         outPath[i++] = inPath[n - 1];
      }
   }
   outPath[i] = 0;

   // ensure path doesn't end in slash
   int slash = strcspn(outPath, "?&") - 1;
   if(slash > 0 && outPath[slash] == '/')
   {
      // shift left to remove slash
      for(; outPath[slash] != 0; slash++)
      {
         outPath[slash] = outPath[slash + 1];
      }
   }
}
