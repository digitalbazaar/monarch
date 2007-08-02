/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpRequestHeader.h"

using namespace std;
using namespace db::net;
using namespace db::net::http;
using namespace db::util;

HttpRequestHeader::HttpRequestHeader()
{
}

HttpRequestHeader::~HttpRequestHeader()
{
}

bool HttpRequestHeader::parseStartLine(const char* str, unsigned int length)
{
   // copy string so it can be modified
   char tokens[length + 1];
   strncpy(tokens, str, length);
   memset(tokens + length, 0, 1);
   
   // find space-delimited tokens in the passed string
   int count = 0;
   char* start = tokens;
   char* end;
   while(start != NULL && count < 3)
   {
      // find the end of the token
      end = strchr(start, ' ');
      if(end != NULL)
      {
         // nullify delimiter
         memset(end, 0, 1);
      }
      
      switch(count++)
      {
         case 0:
            setMethod(start);
            break;
         case 1:
            setPath(start);
            break;
         case 2:
            setVersion(start);
            break;
      }
      
      if(end != NULL)
      {
         start = end + 1;
      }
      else
      {
         start = end;
      }
   }
   
   return count == 3;
}

void HttpRequestHeader::getStartLine(string& line)
{
   line.append(getMethod());
   line.append(1, ' ');
   line.append(getPath());
   line.append(1, ' ');
   line.append(getVersion());
}

void HttpRequestHeader::setMethod(const string& method)
{
   mMethod = method;
}

const string& HttpRequestHeader::getMethod()
{
   return mMethod;
}

void HttpRequestHeader::setVersion(const string& version)
{
   mVersion = version;
}

const string& HttpRequestHeader::getVersion()
{
   return mVersion;
}

void HttpRequestHeader::setPath(const string& path)
{
   mPath = path;
}

const string& HttpRequestHeader::getPath()
{
   return mPath;
}
