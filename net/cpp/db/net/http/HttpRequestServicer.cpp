/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpRequestServicer.h"

using namespace db::net::http;

HttpRequestServicer::HttpRequestServicer(const char* path)
{
   mPath = (char*)malloc(strlen(path) + 1);
   normalizePath(path, mPath);
}

HttpRequestServicer::~HttpRequestServicer()
{
   // free path
   free(mPath);
}

const char* HttpRequestServicer::getPath()
{
   return mPath;
}

void HttpRequestServicer::normalizePath(const char* inPath, char* outPath)
{
   unsigned int length = strlen(inPath);
   
   if(length == 0)
   {
      outPath[0] = '/';
      outPath[1] = 0;
   }
   else
   {
      // prepend slash as necessary
      if(inPath[0] != '/')
      {
         outPath[0] = '/';
         strcpy(outPath + 1, inPath);
      }
      else
      {
         strcpy(outPath, inPath);
      }
      
      // ensure path doesn't end in slash
      int i = strcspn(outPath, "?&") - 1;
      if(i > 0 && outPath[i] == '/')
      {
         // shift left to remove slash
         for(; outPath[i] != 0; i++)
         {
            outPath[i] = outPath[i + 1];
         }
      }
   }
}
