/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpResponseHeader.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>

using namespace std;
using namespace monarch::http;

HttpResponseHeader::HttpResponseHeader() :
   mStatusCode(0),
   mStatusMessage(NULL)
{
}

HttpResponseHeader::~HttpResponseHeader()
{
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

      ++count;

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

void HttpResponseHeader::setStatus(int code, const char* message)
{
   mStatusCode = code;
   // if not explicit, try to get default for the code
   if(message == NULL)
   {
      message = getDefaultStatusMessage(code);
   }
   // default to empty message
   if(message == NULL)
   {
      message = "";
   }
   free(mStatusMessage);
   mStatusMessage = strdup(message);
}

void HttpResponseHeader::clearStatus()
{
   mStatusCode = 0;
   free(mStatusMessage);
   mStatusMessage = NULL;
}

int HttpResponseHeader::getStatusCode()
{
   return mStatusCode;
}

const char* HttpResponseHeader::getStatusMessage()
{
   return (mStatusMessage == NULL) ? "" : mStatusMessage;
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

const char* HttpResponseHeader::getDefaultStatusMessage(int code)
{
   const char* rval;

   // http://www.iana.org/assignments/http-status-codes
   switch(code)
   {
      case 100: rval = "Continue"; break;
      case 101: rval = "Switching Protocols"; break;
      case 102: rval = "Processing"; break;
      case 200: rval = "OK"; break;
      case 201: rval = "Created"; break;
      case 202: rval = "Accepted"; break;
      case 203: rval = "Non-Authoritative Information"; break;
      case 204: rval = "No Content"; break;
      case 205: rval = "Reset Content"; break;
      case 206: rval = "Partial Content"; break;
      case 207: rval = "Multi-Status"; break;
      case 208: rval = "Already Reported"; break;
      case 226: rval = "IM Used"; break;
      case 300: rval = "Multiple Choices"; break;
      case 301: rval = "Moved Permanently"; break;
      case 302: rval = "Found"; break;
      case 303: rval = "See Other"; break;
      case 304: rval = "Not Modified"; break;
      case 305: rval = "Use Proxy"; break;
      case 307: rval = "Temporary Redirect"; break;
      case 400: rval = "Bad Request"; break;
      case 401: rval = "Unauthorized"; break;
      case 402: rval = "Payment Required"; break;
      case 403: rval = "Forbidden"; break;
      case 404: rval = "Not Found"; break;
      case 405: rval = "Method Not Allowed"; break;
      case 406: rval = "Not Acceptable"; break;
      case 407: rval = "Proxy Authentication Required"; break;
      case 408: rval = "Request Timeout"; break;
      case 409: rval = "Conflict"; break;
      case 410: rval = "Gone"; break;
      case 411: rval = "Length Required"; break;
      case 412: rval = "Precondition Failed"; break;
      case 413: rval = "Request Entity Too Large"; break;
      case 414: rval = "Request-URI Too Long"; break;
      case 415: rval = "Unsupported Media Type"; break;
      case 416: rval = "Requested Range Not Satisfiable"; break;
      case 417: rval = "Expectation Failed"; break;
      case 422: rval = "Unprocessable Entity"; break;
      case 423: rval = "Locked"; break;
      case 424: rval = "Failed Dependency"; break;
      case 426: rval = "Upgrade Required"; break;
      case 500: rval = "Internal Server Error"; break;
      case 501: rval = "Not Implemented"; break;
      case 502: rval = "Bad Gateway"; break;
      case 503: rval = "Service Unavailable"; break;
      case 504: rval = "Gateway Timeout"; break;
      case 505: rval = "HTTP Version Not Supported"; break;
      case 506: rval = "Variant Also Negotiates"; break;
      case 507: rval = "Insufficient Storage"; break;
      case 508: rval = "Loop Detected"; break;
      case 510: rval = "Not Extended"; break;
      default: rval = NULL; break;
   }

   return rval;
}

