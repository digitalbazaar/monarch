/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpHeader.h"

using namespace std;
using namespace db::net::http;

HttpHeader::HttpHeader()
{
}

HttpHeader::~HttpHeader()
{
}

void HttpHeader::setHeader(const string& header, const string& value)
{
}

void HttpHeader::addHeader(const string& header, const string& value)
{
}

bool HttpHeader::getHeader(const string& header, string& value)
{
   bool rval = false;
   
   // biCapitalize header
   string str = header;
   biCapitalize(str);
   
   // find header entry
   map<string, string>::iterator i = mHeaders.find(str);
   if(i != mHeaders.end()) 
   {
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
