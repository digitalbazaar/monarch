/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpRequestServicer.h"

using namespace monarch::http;

HttpRequestServicer::HttpRequestServicer(const char* path) :
   mPath((char*)malloc(strlen(path) + 2))
{
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
   // if the path doesn't start with a '/' then prepend one
   int i = 0;
   if(inPath[0] != '/')
   {
      outPath[i++] = '/';
   }

   // copy inPath to outPath, removing any duplicate slashes before the query
   int end = 0;
   bool query = false;
   for(int n = 1; inPath[n - 1] != 0; ++n)
   {
      if(!query && inPath[n - 1] == '?')
      {
         // query found,
         query = true;
         end = i - 1;
      }

      // copy the path if we've hit the query or if the path doesn't have
      // a double (or more) forward slash (which we are reducing to a single)
      if(query || inPath[n] != '/' || inPath[n - 1] != '/')
      {
         outPath[i++] = inPath[n - 1];
      }
   }
   outPath[i] = 0;

   // ensure path doesn't end in slash
   if(!query)
   {
      end = i - 1;
   }
   if(outPath[end] == '/')
   {
      // shift left to remove slash
      for(; outPath[end] != 0; ++end)
      {
         outPath[end] = outPath[end + 1];
      }
   }
}
