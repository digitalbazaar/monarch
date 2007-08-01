/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpHeader.h"
#include "StringTools.h"

using namespace std;
using namespace db::net::http;
using namespace db::util;

// define CRLF
const char HttpHeader::CRLF[] = "\r\n";

HttpHeader::HttpHeader()
{
}

HttpHeader::~HttpHeader()
{
}

void HttpHeader::setHeader(const string& header, const string& value)
{
   // bicapitalize and trim header
   string bic = header;
   biCapitalize(StringTools::trim(bic));
   
   // trim and set value
   string v = value;
   mHeaders[bic] = StringTools::trim(v);
}

void HttpHeader::addHeader(const string& header, const string& value)
{
   // bicapitalize and trim header
   string bic = header;
   biCapitalize(StringTools::trim(bic));
   
   // get existing value
   string existing = "";
   getHeader(bic, existing);
   
   // trim and append new value
   string v = value;
   setHeader(bic, existing + ", " + StringTools::trim(v));
}

void HttpHeader::removeHeader(const string& header)
{
   // bicapitalize and trim header
   string bic = header;
   biCapitalize(StringTools::trim(bic));
   
   // erase it
   mHeaders.erase(bic);
}

void HttpHeader::clearHeaders()
{
   mHeaders.clear();
}

bool HttpHeader::getHeader(const string& header, string& value)
{
   bool rval = false;
   
   // bicapitalize and trim header
   string bic = header;
   biCapitalize(StringTools::trim(bic));
   
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

bool HttpHeader::parse(const string& str)
{
   // parse start line
   bool rval = parseStartLine(str);
   
   // clear headers
   clearHeaders();
   
   // parse lines according to CRLF (and skip start-line)
   string line;
   string::size_type lineStart = str.find(CRLF);
   string::size_type lineEnd = 0;
   while(lineStart < str.length() - 2 &&
         (lineEnd = str.find(CRLF, lineStart + 2)) != string::npos)
   {
      // get line
      line = str.substr(lineStart + 2, lineEnd - lineStart - 2);
      lineStart = lineEnd;
      
      // parse header
      string::size_type colon = line.find(':');
      if(colon != string::npos)
      {
         if(colon != line.length() - 1)
         {
            setHeader(line.substr(0, colon), line.substr(colon + 1));
         }
         else
         {
            setHeader(line.substr(0, colon), "");
         }
      }
   }
   
   return rval;
}

string& HttpHeader::toString(string& str)
{
   // append the start line and CRLF
   getStartLine(str);
   str.erase();
   str.append(CRLF);
   
   // append all headers
   for(map<string, string>::iterator i = mHeaders.begin();
      i != mHeaders.end(); i++)
   {
      str.append(i->first + ": " + i->second + CRLF);
   }
   
   // add CRLF
   str.append(CRLF);
   
   return str;
}

void HttpHeader::setDate(Date* date)
{
   // get GMT time zone
   TimeZone gmt = TimeZone::getTimeZone("GMT");
   string str;
   string format = "%a, %d %b %Y %H:%M:%S GMT";
   
   if(date == NULL)
   {
      // get current date
      Date now;
      now.format(str, format, "c", &gmt);
   }
   else
   {
      date->format(str, format, "c", &gmt);
   }
   
   // set date header
   setHeader("Date", str);
}

bool HttpHeader::getDate(Date& date)
{
   bool rval = false;
   
   string str;
   if(getHeader("Date", str))
   {
      // get GMT time zone
      TimeZone gmt = TimeZone::getTimeZone("GMT");
      rval = date.parse(str, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
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
