/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpRequestHeader.h"

using namespace std;
using namespace db::net;
using namespace db::net::http;
using namespace db::util;

HttpRequestHeader::HttpRequestHeader()
{
   mMethod = strdup("");
   mVersion = strdup("");
   mPath = strdup("");
}

HttpRequestHeader::~HttpRequestHeader()
{
   free(mMethod);
   free(mVersion);
   free(mPath);
}

bool HttpRequestHeader::parseStartLine(const char* str, unsigned int length)
{
   // copy string so it can be modified
   char tokens[length + 1];
   strncpy(tokens, str, length);
   tokens[length] = 0;
   
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
         end[0] = 0;
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
      
      // increment start
      start = (end != NULL) ? end + 1 : end;
   }
   
   return count == 3;
}

void HttpRequestHeader::getStartLine(string& line)
{
   line.append(getMethod());
   line.push_back(' ');
   line.append(getPath());
   line.push_back(' ');
   line.append(getVersion());
}

void HttpRequestHeader::setMethod(const char* method)
{
   free(mMethod);
   mMethod = strdup(method);
}

const char* HttpRequestHeader::getMethod()
{
   return mMethod;
}

void HttpRequestHeader::setVersion(const char* version)
{
   free(mVersion);
   mVersion = strdup(version);
}

const char* HttpRequestHeader::getVersion()
{
   return mVersion;
}

void HttpRequestHeader::setPath(const char* path)
{
   free(mPath);
   mPath = strdup(path);
}

const char* HttpRequestHeader::getPath()
{
   return mPath;
}
