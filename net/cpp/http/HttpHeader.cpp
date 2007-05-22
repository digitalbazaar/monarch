/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpHeader.h"

using namespace std;
using namespace db::net::http;

// define CRLF
const char CRLF[] = "\r\n";

HttpHeader::HttpHeader()
{
}

HttpHeader::~HttpHeader()
{
}

void HttpHeader::setHeader(const string& header, const string& value)
{
   // bicapitalize header
   string bic = header;
   biCapitalize(bic);
   
   // insert header
   mHeaders[bic] = value;
}

void HttpHeader::addHeader(const string& header, const string& value)
{
   // bicapitalize header
   string bic = header;
   biCapitalize(bic);
   
   // get existing value
   string existing = "";
   getHeader(bic, existing);
   
   // append new value
   setHeader(bic, existing + ", " + value);
}

void HttpHeader::removeHeader(const string& header)
{
   // bicapitalize header
   string bic = header;
   biCapitalize(bic);
   
   // erase it
   mHeaders.erase(bic);
}

bool HttpHeader::getHeader(const string& header, string& value)
{
   bool rval = false;
   
   // bicapitalize header
   string bic = header;
   biCapitalize(bic);
   
   // find header entry
   map<string, string>::iterator i = mHeaders.find(bic);
   if(i != mHeaders.end()) 
   {
      // get value
      value = i->second;
      rval = true;
   }
   
   return rval;
}

void HttpHeader::biCapitalize(string& header)
{
   // capitalize the first letter
   transform(header.begin(), header.begin() + 1, header.begin(), toupper);
   
   // decapitalize all other letters
   transform(header.begin() + 1, header.end(), header.begin() + 1, tolower);
   
   // capitalize all other letters that occur after hyphens
   for(string::iterator i = header.begin() + 1; i != header.end(); i++)
   {
      if(*(i - 1) == '-')
      {
         // capitalize the letter
         transform(i, i + 1, i, toupper);
      }
   }
}
