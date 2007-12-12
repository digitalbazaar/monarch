/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpResponseHeader.h"
#include "db/util/StringTokenizer.h"

using namespace std;
using namespace db::net::http;
using namespace db::util;

HttpResponseHeader::HttpResponseHeader()
{
   mVersion = strdup("");
   mStatusCode = 0;
   mStatusMessage = strdup("");
}

HttpResponseHeader::~HttpResponseHeader()
{
   free(mVersion);
   free(mStatusMessage);
}

bool HttpResponseHeader::parseStartLine(const char* str, unsigned int length)
{
   bool rval = false;
   
   // create status message string
   unsigned int msgLength = 0;
   char msg[length];
   msg[0] = 0;
   
   // copy string so it can be modified
   char tokens[length + 1];
   strncpy(tokens, str, length);
   tokens[length] = 0;
   
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
         end[0] = 0;
      }
      
      if(count == 0)
      {
         setVersion(start);
      }
      else if(count == 1)
      {
         mStatusCode = strtoul(start, NULL, 10);
         rval = true;
      }
      else
      {
         if(msgLength > 0)
         {
            msg[msgLength++] = ' ';
         }
         
         strcpy(msg + msgLength, start);
         msgLength += (end - start);
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
   
   // set status message
   free(mStatusMessage);
   mStatusMessage = strdup(msg);
   
   return rval;
}

void HttpResponseHeader::getStartLine(string& line)
{
   char code[11];
   sprintf(code, "%d", getStatusCode());
   line.append(getVersion());
   line.push_back(' ');
   line.append(code);
   line.push_back(' ');
   line.append(getStatusMessage());
}

void HttpResponseHeader::setVersion(const char* version)
{
   free(mVersion);
   mVersion = strdup(version);
}

const char* HttpResponseHeader::getVersion()
{
   return mVersion;
}

void HttpResponseHeader::setStatus(unsigned int code, const char* message)
{
   mStatusCode = code;
   
   free(mStatusMessage);
   mStatusMessage = strdup(message);
}

unsigned int HttpResponseHeader::getStatusCode()
{
   return mStatusCode;
}

const char* HttpResponseHeader::getStatusMessage()
{
   return mStatusMessage;
}
