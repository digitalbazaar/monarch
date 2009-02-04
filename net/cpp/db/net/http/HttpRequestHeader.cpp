/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpRequestHeader.h"

#include <cstdlib>

using namespace std;
using namespace db::net;
using namespace db::net::http;
using namespace db::util;

HttpRequestHeader::HttpRequestHeader()
{
   mMethod = NULL;
   mPath = NULL;
}

HttpRequestHeader::~HttpRequestHeader()
{
   if(mMethod != NULL)
   {
      free(mMethod);
   }
   
   if(mPath != NULL)
   {
      free(mPath);
   }
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
   // METHOD PATH HTTP/1.1
   int length = 20 + strlen(getPath());
   char tmp[length];
   snprintf(tmp, length, "%s %s %s", mMethod, mPath, mVersion);
   line.append(tmp);
}

inline bool HttpRequestHeader::hasStartLine()
{
   // has start line
   return true;
}

void HttpRequestHeader::setMethod(const char* method)
{
   if(mMethod != NULL)
   {
      free(mMethod);
   }
   mMethod = strdup(method);
}

const char* HttpRequestHeader::getMethod()
{
   if(mMethod == NULL)
   {
      mMethod = strdup("");
   }
   return mMethod;
}

void HttpRequestHeader::setPath(const char* path)
{
   if(mPath != NULL)
   {
      free(mPath);
   }
   mPath = strdup(path);
}

const char* HttpRequestHeader::getPath()
{
   if(mPath == NULL)
   {
      mPath = strdup("");
   }
   return mPath;
}

void HttpRequestHeader::writeTo(HttpRequestHeader* header)
{
   HttpHeader::writeTo(header);
   header->setMethod(getMethod());
   header->setPath(getPath());
}
