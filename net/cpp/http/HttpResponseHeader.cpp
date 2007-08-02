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
   
   // tokenize on spaces
   StringTokenizer st(str.c_str(), ' ');
   if(st.getTokenCount() == 3)
   {
      setVersion(st.nextToken());
      long long code;
      if(Convert::stringToInteger(st.nextToken(), code))
      {
         string msg;
         while(st.hasNextToken())
         {
            if(msg.length() == 0)
            {
               msg.append(st.nextToken());
            }
            else
            {
               msg.append(" ");
               msg.append(st.nextToken());
            }
         }
         
         setStatus(code, msg);
         rval = true;
      }
      else
      {
         setStatus(0, "");
      }
   }
   else
   {
      setVersion("");
      setStatus(0, "");
   }
   
   return rval;
}

void HttpResponseHeader::getStartLine(string& line)
{
   char code[10];
   sprintf(code, "%d", getStatusCode()); 
   line.append(getVersion() + " " + code + " " + getStatusMessage());
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
