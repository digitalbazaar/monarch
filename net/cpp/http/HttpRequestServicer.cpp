/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpRequestServicer.h"

using namespace std;
using namespace db::net::http;

HttpRequestServicer::HttpRequestServicer(const string& path)
{
   if(path.length() == 0)
   {
      mPath = '/';
   }
   else
   {
      // prepend slash as necessary
      if(mPath[0] != '/')
      {
         mPath.insert(0, 1, '/');
      }
      
      // append slash as necessary
      if(mPath[mPath.length() - 1] != '/')
      {
         mPath.append(1, '/');
      }
   }
}

HttpRequestServicer::~HttpRequestServicer()
{
}

const string& HttpRequestServicer::getPath()
{
   return mPath;
}
