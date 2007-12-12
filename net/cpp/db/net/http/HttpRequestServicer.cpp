/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpRequestServicer.h"

using namespace std;
using namespace db::net::http;

HttpRequestServicer::HttpRequestServicer(const char* path)
{
   unsigned int length = strlen(path);
   
   if(length == 0)
   {
      mPath = (char*)malloc(2);
      mPath[0] = '/';
      mPath[1] = 0;
   }
   else
   {
      // prepend/append slashes as necessary
      if(path[0] != '/' && length > 1)
      {
         if(path[length - 1] != '/')
         {
            mPath = (char*)malloc(length + 3);
            sprintf(mPath, "/%s/", path);
         }
         else
         {
            mPath = (char*)malloc(length + 2);
            sprintf(mPath, "/%s", path);
         }
      }
      else if(path[length - 1] != '/')
      {
         mPath = (char*)malloc(length + 2);
         sprintf(mPath, "%s/", path);
      }
      else
      {
         mPath = (char*)malloc(length + 1);
         strcpy(mPath, path);
      }
   }
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
