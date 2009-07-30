/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/http/HttpResponseHeader.h"

#include <cstdlib>
#include <cstring>

using namespace std;
using namespace db::http;

HttpResponseHeader::HttpResponseHeader()
{
   mStatusCode = 0;
   mStatusMessage = NULL;
}

HttpResponseHeader::~HttpResponseHeader()
{
   if(mStatusMessage != NULL)
   {
      free(mStatusMessage);
   }
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
   if(mStatusMessage != NULL)
   {
      free(mStatusMessage);
   }
   mStatusMessage = strdup(msg);
   
   return rval;
}

void HttpResponseHeader::getStartLine(string& line)
{
   // HTTP/1.1 XXX status message
   int length = 20 + strlen(getStatusMessage());
   char tmp[length];
   snprintf(tmp, length, "%s %d %s", mVersion, mStatusCode, mStatusMessage);
   line.append(tmp);
}

inline bool HttpResponseHeader::hasStartLine()
{
   // has start line
   return true;
}

void HttpResponseHeader::setStatus(unsigned int code, const char* message)
{
   mStatusCode = code;
   
   if(mStatusMessage != NULL)
   {
      free(mStatusMessage);
   }
   mStatusMessage = strdup(message);
}

void HttpResponseHeader::clearStatus()
{
   mStatusCode = 0;
   if(mStatusMessage != NULL)
   {
      free(mStatusMessage);
      mStatusMessage = NULL;
   }
}

unsigned int HttpResponseHeader::getStatusCode()
{
   return mStatusCode;
}

const char* HttpResponseHeader::getStatusMessage()
{
   if(mStatusMessage == NULL)
   {
      mStatusMessage = strdup("");
   }
   return mStatusMessage;
}

void HttpResponseHeader::writeTo(HttpResponseHeader* header)
{
   HttpHeader::writeTo(header);
   header->setStatus(getStatusCode(), getStatusMessage());
}

inline HttpHeader::Type HttpResponseHeader::getType()
{
   return HttpHeader::Response;
}
