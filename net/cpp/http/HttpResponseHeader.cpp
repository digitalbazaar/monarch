/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpResponseHeader.h"
#include "StringTokenizer.h"
#include "Convert.h"

using namespace std;
using namespace db::net::http;
using namespace db::util;

HttpResponseHeader::HttpResponseHeader()
{
   mStatusCode = 0;
}

HttpResponseHeader::~HttpResponseHeader()
{
}

bool HttpResponseHeader::parseStartLine(const std::string& str)
{
   bool rval = false;
   
   // clean status message
   mStatusMessage.erase();
   
   // copy string so it can be modified
   char tokens[str.length() + 1];
   strcpy(tokens, str.c_str());
   
   // find space-delimited tokens in the passed string
   int count = 0;
   char* start = tokens;
   char* end;
   while(start != NULL)
   {
      // find the end of the token
      end = strchr(start, ' ');
      if(end != NULL)
      {
         // nullify delimiter
         memset(end, 0, 1);
      }
      
      if(count == 0)
      {
         setVersion(start);
      }
      else if(count == 1)
      {
         long long code;
         if(Convert::stringToInteger(start, code))
         {
            mStatusCode = code;
            rval = true;
         }
      }
      else
      {
         if(mStatusMessage.length() > 0)
         {
            mStatusMessage.append(1, ' ');
         }
         
         mStatusMessage.append(start);
      }
      
      count++;
      
      if(end != NULL)
      {
         start = end + 1;
      }
      else
      {
         start = end;
      }
   }
   
   return rval;
}

void HttpResponseHeader::getStartLine(string& line)
{
   char code[10];
   sprintf(code, "%d", getStatusCode()); 
   line.append(getVersion());
   line.append(1, ' ');
   line.append(code);
   line.append(1, ' ');
   line.append(getStatusMessage());
}

void HttpResponseHeader::setVersion(const string& version)
{
   mVersion = version;
}

const string& HttpResponseHeader::getVersion()
{
   return mVersion;
}

void HttpResponseHeader::setStatus(unsigned int code, const string& message)
{
   mStatusCode = code;
   mStatusMessage = message;
}

unsigned int HttpResponseHeader::getStatusCode()
{
   return mStatusCode;
}

const string& HttpResponseHeader::getStatusMessage()
{
   return mStatusMessage;
}
